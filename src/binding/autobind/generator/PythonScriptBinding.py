import pybindgen
import sys
import io
import os

def Generate(cxxRootNode):
    from ..cppmodel.CxxNode import Generator
    from ..cppmodel.CxxFunction import CxxFunction
    from ..cppmodel.CxxClass import CxxClass
    from ..cppmodel.CxxVariable import CxxVariable

    filePath = str(cxxRootNode)
    commonPrefix = os.path.commonprefix([filePath, __file__])
    relativePath = os.path.relpath(filePath, commonPrefix)
    fileBasePath, fileExt = os.path.splitext(relativePath)
    fileModuleName = fileBasePath.replace(os.path.sep, "_")

    pybindMod = pybindgen.Module(fileModuleName)
    # pybindMod.add_include('"shared/cube.h"')

    amountOfSymbolsAdded = 0;

    for node in cxxRootNode.forEachChild():
        if type(node) is CxxFunction:
            if node.generateFor & Generator.Python:
                amountOfSymbolsAdded += GenerateFunction(pybindMod, node)
        if type(node) is CxxClass:
            if node.generateFor & Generator.Python:
                amountOfSymbolsAdded += GenerateClass(node)
        if type(node) is CxxVariable:
            if node.generateFor & Generator.Python:
                amountOfSymbolsAdded += GenerateVariable(node)

    if amountOfSymbolsAdded > 0:
        output = io.StringIO()
        pybindMod.generate(output)

        return output.getvalue()
    else:
        return ""

def CommandTypeToCType(commandType):
    commandTypeMap = {
        "CommandTypes::KeyPress": "int *",
        "CommandTypes::Integer": "int *",
        "CommandTypes::Boolean": "int *",
        "CommandTypes::ArgLen": "int *",
        "CommandTypes::Expression": "uint *",
        "CommandTypes::OptionalFloat": "float *"
    }

    return commandTypeMap[commandType]

def ArgumentTypeToCType(argType):
    if argType.startswith("CommandTypes::"):
        argType = CommandTypeToCType(argType)

    typeMap = {
        "uchar": "unsigned char",
        "ushort": "unsigned short",
        "uint": "unsigned int",
        "ulong": "unsigned long",
        "llong": "signed long long int",
        "ullong": "unsigned long long int",
    }

    for type, replacement in typeMap.items():
        if argType.startswith(type):
            argType = argType.replace(type, replacement)

    return argType

def ArgumentToParameter(arg):
    argType = arg.type.spelling
    argName = arg.spelling

    return pybindgen.param(ArgumentTypeToCType(argType), argName, direction=pybindgen.Parameter.DIRECTION_IN)


def GenerateFunction(module, node):
    functionName = node.symbolName()
    namespace = node.cxxNamespace()
    returnType = node.cxxReturnType()
    if namespace:
        for ns in node.cxxNamespaceList():
            module = module.add_cpp_namespace(ns)
    if returnType == "void":
        returnType = None
    else:
        returnType = pybindgen.retval(returnType)
    arguments = []
    for arg in node.forEachArgumentNameType():
        arguments.append(ArgumentToParameter(arg))

    try:
        if node.customFuctionName:
            module.add_function(node.sourceObject.spelling, returnType, arguments, custom_name=functionName)
        else:
            module.add_function(functionName, returnType, arguments)
        return 1
    except pybindgen.typehandlers.base.TypeLookupError as e:
        print(
            ">>> Unable to generate Python binding for function {}: Unknown type {}".format(
                functionName,
                str(e).replace("pybindgen.typehandlers.base.TypeLookupError: ", "")
            ),
            file=sys.stderr
        )
        print(
            """>>> WARN: Ignored function {}: it will not be accessible in Python
>>>       Prepend the function with
>>>           SCRIPTBIND_OPT(BINDOPT_GENERATORS, BINDOPER_DROP, BINDGENERATOR_PYTHON)
>>>       to ignore this message""".format(
                functionName,
            ),
            file=sys.stderr
        )

    return 0

def GenerateClass(node):
    return 0

def GenerateVariable(node):
    return 0
