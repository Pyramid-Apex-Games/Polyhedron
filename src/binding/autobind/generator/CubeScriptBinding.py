import json
# from ..cppmodel.CxxNode import CxxNode
# from ..cppmodel.CxxInclude import CxxInclude

def Generate(cxxRootNode):
    from ..cppmodel.CxxNode import Generator
    from ..cppmodel.CxxFunction import CxxFunction

    template = """#include "shared/cube.h"
{}"""
    generated_funcs = []
    for node in cxxRootNode.forEachChild():
        if type(node) is CxxFunction:
            if node.generateFor & Generator.CubeScript:
                generated_funcs.append(GenerateFunction(node))
                generated_funcs.append(GenerateFunctionForwardDeclaration(node))
                generated_funcs.append(GenerateFunctionImplementation(node))
                generated_funcs.append(GenerateFunctionDebug(node))
    if len(generated_funcs) == 0:
        return ""
    return template.format("\n".join(generated_funcs))


def GenerateWithoutMacros(cxxRootNode):
    from ..cppmodel.CxxNode import Generator
    from ..cppmodel.CxxFunction import CxxFunction
    
    template = """// === GENERATED FILE, EDITS WILL NOT STICK === //
#include "engine/scriptexport.h"
#include "shared/cube.h"

{forwarddecls}

{implementations}

{debugs}
"""
    forwarddecls = []
    implementations = []
    debugs = []
    for node in cxxRootNode.forEachChild():
        if type(node) is CxxFunction:
            if node.generateFor & Generator.CubeScript:
                forwarddecls.append(GenerateFunctionForwardDeclaration(node))
                implementations.append(GenerateFunctionImplementation(node))
                debugs.append(GenerateFunctionDebug(node))
    if len(forwarddecls) == 0:
        return ""
    return template.format(
        forwarddecls = "\n".join(forwarddecls),
        implementations = "\n".join(implementations),
        debugs = "\n".join(debugs)
    )

def GetCubeScriptPrototype(cxxFunc):
    spelling2proto  = {
        "int": "i",
        "int *": "i",
        "float": "f",
        "float *": "f",
        "bool": "b",
        "const char *": "s",
        "char *": "s",
        "CommandTypes::KeyPress": "D",
        "CommandTypes::Boolean": "b",
        "ident *": "r",
        "CommandTypes::ArgLen": "N",
        "CommandTypes::Expression": "e",
        "CommandTypes::OptionalFloat": "F",
        "tagval *": "t",
    }

    proto = ""
    for arg in cxxFunc.forEachArgument():
        if (arg.spelling in spelling2proto):
            proto = proto + spelling2proto[arg.spelling]
        else:
            raise ValueError("Function argument type not implemented", arg.spelling, cxxFunc.sourceObject.spelling)

    return proto

def GetCppArgumentsTypes(cxxFunc):
    cppargs = []
    for arg in cxxFunc.forEachArgument():
        cppargs.append(arg.spelling)

    return cppargs

def GetCppArgumentsNames(cxxFunc):
    argname = []
    argnameitr = 0
    for arg in cxxFunc.forEachArgument():
        argname.append(chr(97 + argnameitr))
        argnameitr = argnameitr + 1        

    return argname

def GenerateFunction(cxxFunc):
#         template = """COMMAND({functionname}, "{proto}", {doc});
# """
#         template_custom = """ICOMMAND({functionname}, "{proto}", ({argdecl}), {realfunctionname}({arguse}), {doc});
# """

    template = """{returnt} {functionname}({argdecl});
COMMAND({functionname}, "{proto}", {doc});
"""
    template_custom = """namespace {namespace}{{ {returnt} {functionnamebound}({argdecl}); }}
ICOMMAND({functionname}, "{proto}", ({argdecl}), {realfunctionname}({arguse}), {doc});
"""
    template_custom_nns = """{returnt} {realfunctionname}({argdecl});
ICOMMAND({functionname}, "{proto}", ({argdecl}), {realfunctionname}({arguse}), {doc});
"""

    proto = GetCubeScriptPrototype(cxxFunc)
    cppargs = GetCppArgumentsTypes(cxxFunc)
    argname = GetCppArgumentsNames(cxxFunc)

    arguselst = []
    for [a, b] in zip(cppargs, argname):
        arguselst.append(a + " " + b)

    argdecl = ", ".join(cppargs)
    argusedecl = ", ".join(arguselst)
    arguse = ", ".join(argname)
    functionName = cxxFunc.sourceObject.spelling
    returntype = cxxFunc.sourceObject.result_type.spelling
    namespace = "::".join(cxxFunc.getContainingNamespaces(cxxFunc.sourceObject, []))
    realFunctionName = ""
    if namespace:
        realFunctionName = namespace + "::"

    realFunctionName = realFunctionName + functionName
    functionNameBound = functionName

    if cxxFunc.customFuctionName or namespace:
        if cxxFunc.customFuctionName:
            functionName = cxxFunc.customFuctionName
        
        if namespace:
            return template_custom.format(
                argdecl = argusedecl,
                arguse = arguse,
                realfunctionname = realFunctionName,
                functionname = functionName,
                functionnamebound = functionNameBound,
                namespace = namespace,
                proto = proto,
                returnt = returntype,
                doc = json.dumps(str(cxxFunc.comment()))
            )
        else:
            return template_custom_nns.format(
                argdecl = argusedecl,
                arguse = arguse,
                realfunctionname = realFunctionName,
                functionname = functionName,
                namespace = namespace,
                proto = proto,
                returnt = returntype,
                doc = json.dumps(str(cxxFunc.comment()))
            )
    else:
        return template.format(
            argdecl = argdecl,
            functionname = functionName,
            realfunctionname = realFunctionName,
            namespace = namespace,
            proto = proto,
            returnt = returntype,
            doc = json.dumps(str(cxxFunc.comment()))
        )

def GenerateFunctionTemplateValues(cxxFunc):
    cubeScriptProto = GetCubeScriptPrototype(cxxFunc)
    cppArgumentTypes = GetCppArgumentsTypes(cxxFunc)
    cppArgumentNames = GetCppArgumentsNames(cxxFunc)
    cppArgumentDecls = []
    for [a, b] in zip(cppArgumentTypes, cppArgumentNames):
        cppArgumentDecls.append(a + " " + b)

    cppOriginalFunctionName = cxxFunc.sourceObject.spelling
    cppReturnType = cxxFunc.sourceObject.result_type.spelling
    cppNamespaceHierarchy = cxxFunc.getContainingNamespaces(cxxFunc.sourceObject, [])
    documentComment = json.dumps(str(cxxFunc.comment()))

    if cxxFunc.customFuctionName:
        bindWithFunctionName = cxxFunc.customFuctionName
    else:
        bindWithFunctionName = cppOriginalFunctionName

    return {
        "cubeScriptProto": cubeScriptProto,
        "cppArgumentTypes": cppArgumentTypes,
        "cppArgumentNames": cppArgumentNames,
        "cppArgumentDecls": cppArgumentDecls,
        "cppOriginalFunctionName": cppOriginalFunctionName,
        "cppReturnType": cppReturnType,
        "cppNamespaceHierarchy": cppNamespaceHierarchy,
        "bindWithFunctionName": bindWithFunctionName,
        "documentComment": documentComment        
    }  

def GenerateFunctionForwardDeclaration(cxxFunc):
    templateValues = GenerateFunctionTemplateValues(cxxFunc)
    vals = {
        "return": templateValues['cppReturnType'],
        "args": ", ".join(templateValues['cppArgumentDecls']),
        "func": templateValues['cppOriginalFunctionName']
    }

    forwardDecl = "extern {return} {func}({args});".format(**vals)
    if templateValues['cppNamespaceHierarchy']:
        for ns in templateValues['cppNamespaceHierarchy']:
            forwardDecl = "namespace {} {{ {} }}".format(ns, forwardDecl)

    return forwardDecl

def GenerateFunctionImplementation(cxxFunc):
    templateValues = GenerateFunctionTemplateValues(cxxFunc)
    vals = {
        "boundFunc": templateValues['bindWithFunctionName'],
        "func": templateValues['cppOriginalFunctionName'],
        "proto": templateValues['cubeScriptProto'],
        "doc": templateValues['documentComment']
    }

    if templateValues['cppNamespaceHierarchy']:
        for ns in templateValues['cppNamespaceHierarchy']:
            vals['func'] = "{}::{}".format(ns, vals['func'])

    implementation = "static bool __dummy_{boundFunc}_bool = addcommand(\"{boundFunc}\", (identfun){func}, \"{proto}\", ID_COMMAND, {doc});\n".format(**vals)

    return implementation

def GenerateFunctionDebug(cxxFunc):
    templateValues = GenerateFunctionTemplateValues(cxxFunc)

    return """
    // cubeScriptProto = {cubeScriptProto}
    // cppArgumentTypes = {cppArgumentTypes}
    // cppArgumentNames = {cppArgumentNames}
    // cppArgumentDecls = {cppArgumentDecls}
    // cppOriginalFunctionName = {cppOriginalFunctionName}
    // cppReturnType = {cppReturnType}
    // cppNamespaceHierarchy = {cppNamespaceHierarchy}
    // bindWithFunctionName = {bindWithFunctionName}
    """.format(**templateValues)
    