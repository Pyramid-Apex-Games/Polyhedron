import re

CUSTOM_TOJSON_VARGENERATORS = {}
CUSTOM_FROMJSON_VARGENERATORS = {}
attributeTypes = ["std::string", "float", "int", "bool", "vec4", "vec", "ivec4", "ivec"]

def Generate(cxxRootNode):
    from ..cppmodel.CxxNode import Generator
    from ..cppmodel.CxxClass import CxxClass

    template = """#include "{}"
#include <nlohmann/json.hpp>

{}"""
    generated_funcs = []
    for node in cxxRootNode.forEachChild(noDepth = False):
        if type(node) is CxxClass:
            if node.generateFor & Generator.Json:
                generated_funcs.append(GenerateFromJson(node))
                generated_funcs.append(GenerateToJson(node))
            if node.generateFor & Generator.Attributes:
                generated_funcs.append(GenerateAttributeDefinition(node))
                generated_funcs.append(GenerateAttributeGetter(node))
                generated_funcs.append(GenerateAttributeSetter(node))
            if node.generateFor & Generator.Json:
                generated_funcs.append(GenerateJsonDebug(node))
    if len(generated_funcs) == 0:
        return ""

    headerFile = str(cxxRootNode)[:-4] + ".h"
    return template.format(headerFile, "\n".join(generated_funcs))

def GenerateJsonVariableTemplateValues(cxxVar):
    variableName = cxxVar.sourceObject.spelling
    variableType = cxxVar.sourceObject.type.spelling

    return {
        "variableName": variableName,
        "variableType": variableType
    }

def GenerateJsonTemplateValues(cxxClass):
    className = str(cxxClass)

    return {
        "className": className
    }

def GenerateToJson(cxxClass):
    from ..cppmodel.CxxVariable import CxxVariable
    templateValues = GenerateJsonTemplateValues(cxxClass)

    output = []
    for child in cxxClass.forEachChild(noDepth = True):
        if type(child) == CxxVariable:
            output = output + GenerateToJsonVariable(child, "entity_t")
    body = ",\n\t\t".join(output)
    return cxxClass.NamespaceBlock(f"""void to_json(nlohmann::json& document, const {templateValues['className']}& entity_t)
{{
\tdocument = {{
\t\t{body}
\t}};
}}
""")    

def GenerateToJsonVariable(cxxVar, instanceVar):
    templateValues = GenerateJsonVariableTemplateValues(cxxVar)
    output = []

    if templateValues['variableType'] in CUSTOM_TOJSON_VARGENERATORS:
        output = output + CUSTOM_TOJSON_VARGENERATORS[templateValues['variableType']](templateValues)
    else:
        output.append(f"{{\"{templateValues['variableName']}\", {instanceVar}.{templateValues['variableName']}}}")
    return output


def GenerateFromJson(cxxClass):
    from ..cppmodel.CxxVariable import CxxVariable
    templateValues = GenerateJsonTemplateValues(cxxClass)

    output = []
    for child in cxxClass.forEachChild(noDepth = True):
        if type(child) == CxxVariable:
            output = output + GenerateFromJsonVariable(child, "document", "entity_t")
    body = "\n\t".join(output)
    return cxxClass.NamespaceBlock(f"""void from_json(const nlohmann::json& document, {templateValues['className']}& entity_t)
{{
\t{body}
}}
""")

def GenerateFromJsonVariable(cxxVar, jsonRootVar, instanceVar):
    templateValues = GenerateJsonVariableTemplateValues(cxxVar)
    output = []

    if cxxVar.annotation and cxxVar.annotation.spelling.startswith("dontunserialize"): # <- todo: get this value from parsecpp.py
        return output

    if templateValues['variableType'] in CUSTOM_FROMJSON_VARGENERATORS:
        output = output + CUSTOM_FROMJSON_VARGENERATORS[templateValues['variableType']](templateValues)
    else:
        output.append(f"""if (document.find(\"{templateValues['variableName']}\") != document.end()) 
\t{{
\t\t{jsonRootVar}.at(\"{templateValues['variableName']}\").get_to({instanceVar}.{templateValues['variableName']});
\t}}
""")
    return output


def GenerateAttributeDefinition(cxxClass):
    templateValues = GenerateJsonTemplateValues(cxxClass)

    output = []
    for child in cxxClass.forEachChild(noDepth = True):
        output = output + GenerateAttributeDefinitionVariable(child)
    if len(output) > 0:
        output = [f"{{\"header\"s, \"{cxxClass}\"s}}"] + output
    body = ",\n\t\t".join(output)
    return f"""const AttributeList_T {templateValues['className']}::Attributes()
{{
    using namespace std::string_literals;

    return AttributeList_T {{
        {body}
    }};
}}
"""

def GenerateAttributeDefinitionVariable(cxxVar):
    from ..cppmodel.CxxVariable import CxxVariable
    templateValues = GenerateJsonVariableTemplateValues(cxxVar)
    output = []
    definition = []

    for child in cxxVar.forEachChild(noDepth = True):
        if hasattr(child, 'data'):
            phui, uiType, *data = child.data
            definition.append(f"\"{uiType}\"s")
            definition.append(f"\"{templateValues['variableName']}\"s")
            for element in data:
                definition.append(GenerateAttributeDefinitionVariableSanitized(element))

    if len(definition) == 0:
        if type(cxxVar) is CxxVariable:
            if templateValues['variableType'] in attributeTypes:
                definition.append(f"\"generic_prop\"s")
                definition.append(f"\"{templateValues['variableName']}\"s")
                definition.append(f"\"{MakeVariableHumanReadable(templateValues['variableName'])}\"s")

    if len(definition) > 0:
        output.append(
            "{" + ", ".join(definition) + "}"
        )

    return output

def MakeVariableHumanReadable(variableName):
    if variableName.startswith("m_"):
        variableName = variableName[2:]

    if variableName.find("_"):
        variableName = variableName.replace("_", "")

    words = []
    regex = r"([A-Z]{2,})([A-Z][a-z]+)|([A-Z]?[a-z]+)"
    matches = re.finditer(regex, variableName)
    for matchNum, match in enumerate(matches, start=1):
        for groupNum in range(0, len(match.groups())):
            groupNum = groupNum + 1
            if match.group(groupNum):
                words.append(match.group(groupNum))

    for w in range(0, len(words)):
        if not words[w] in ["is", "a", "for", "of", "in", "by", "not", "the", "an"]:
            words[w] = words[w].capitalize()
        else:
            words[w] = words[w].lower()

    words[0] = words[0].capitalize()

    return " ".join(words)

def GenerateAttributeDefinitionVariableSanitized(element):
    if re.match('^-?[0-9.]+f?$', element):
        if re.match('[^.]', element):
            if element.endswith("f"):
                element = element[0:-1]
            return str(float(element)) + "f"
        else:
            return str(int(element))
    if re.match('^[a-zA-Z][a-zA-Z0-9]*', element):
        return f"\"{element}\"s"
    if element.startswith("\"") and element.endswith("\""):
        return f"{element}s"
    else:
        return f"\"{element}\"s"

def GenerateAttributeGetter(cxxClass):
    from ..cppmodel.CxxVariable import CxxVariable
    templateValues = GenerateJsonTemplateValues(cxxClass)
    hasElse = ""

    output = []
    for child in cxxClass.forEachChild(noDepth = True):
        if type(child) is CxxVariable:
            childTemplateValues = GenerateJsonVariableTemplateValues(child)
            if childTemplateValues['variableType'] in attributeTypes:

                output.append(f"""{hasElse}if (key == "{childTemplateValues['variableName']}"s)
	{{
		return {childTemplateValues['variableName']};
	}}""")

                if len(output) > 0:
                    hasElse = "else "

    body = ""
    if len(output) != 0:
        body = "\n\t".join(output)
    return f"""Attribute_T {templateValues['className']}::GetAttributeImpl(const std::string &key) const
{{
\tusing namespace std::string_literals;

\t{body}

\treturn Attribute_T();
}}
"""

def GenerateAttributeSetter(cxxClass):
    from ..cppmodel.CxxVariable import CxxVariable
    templateValues = GenerateJsonTemplateValues(cxxClass)
    hasElse = ""

    output = []
    for child in cxxClass.forEachChild(noDepth = True):
        if type(child) is CxxVariable:
            childTemplateValues = GenerateJsonVariableTemplateValues(child)
            if childTemplateValues['variableType'] in attributeTypes:
                output.append(f"""{hasElse}if (key == "{childTemplateValues['variableName']}"s)
	{{
        if (std::holds_alternative<{childTemplateValues['variableType']}>(value))
        {{
		    {childTemplateValues['variableName']} = std::get<{childTemplateValues['variableType']}>(value);
        }}
        else
        {{
			{childTemplateValues['variableName']} = std::visit(AttributeVisitCoercer<{childTemplateValues['variableType']}>(), value);
        }}
        OnImpl(changedEvent);
	}}""")

                if len(output) > 0:
                    hasElse = "else "

    body = "\n\t".join(output)
    return f"""void {templateValues['className']}::SetAttributeImpl(const std::string &key, const Attribute_T &value)
{{
\tusing namespace std::string_literals;

\tEntityEventAttributeChanged changedEvent(key);
\t{body}
}}
"""

def GenerateJsonDebug(cxxClass):
    debug_output = []
    for child in cxxClass.forEachChild():
        debug_output.append(GenerateJsonVariableDebug(child))
    
    return "\n".join(debug_output)

def GenerateJsonVariableDebug(cxxVar):
    templateValues = GenerateJsonVariableTemplateValues(cxxVar)

    output = ["// type:  " + str(type(cxxVar))]
    for key in templateValues:
        output.append(f"// {key} => {templateValues[key]}")
    return "\n".join(output)

# ---- Custom Generators for specific variable types

# def CustomVarGenerator_toJson_vec(templateValues):
#     output_obj = []
#     for m in ['x', 'y', 'z']:
#         output_obj.append(f"\"{m}\", {templateValues['variableName']}.{m}")
#     return [f"\"{templateValues['variableName']}\""] + ["{{" + ", ".join(output_obj) + "}}"]

# def CustomVarGenerator_toJson_vec4(templateValues):
#     output_obj = []
#     for m in ['x', 'y', 'z', 'w']:
#         output_obj.append(f"\"{m}\", {templateValues['variableName']}.{m}")
#     return [f"\"{templateValues['variableName']}\""] + ["{{" + ", ".join(output_obj) + "}}"]

# CUSTOM_TOJSON_VARGENERATORS['vec'] = CustomVarGenerator_toJson_vec
# CUSTOM_TOJSON_VARGENERATORS['vec4'] = CustomVarGenerator_toJson_vec4