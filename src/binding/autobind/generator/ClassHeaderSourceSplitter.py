from clang import cindex
import json
import os
import shutil
import glob
import filecmp
from ..clang_format import formatter

OPT_INDENTION_LEVEL = 4
OPT_INDENTION_CHAR = " "

def Generate(cxxRootNode, className, baseFolder = "", baseName = "", testDrive = True):
    from ..cppmodel.CxxTranslationUnit import CxxTranslationUnit
    from ..cppmodel.CxxClass import CxxClass

    if not baseName:
        baseName = className

    if not type(cxxRootNode) is CxxTranslationUnit:
        raise RuntimeError(f"ClassHeaderSourceSplitter: Generate: rootNode is not a CxxTranslationUnit: It's a {str(type(cxxRootNode))}")

    for node in cxxRootNode.forEachChild(noDepth = True):
        if type(node) is CxxClass:
            if str(node) == className:
                headerBasePath = baseName + ".h"

                headerPath = os.path.join(baseFolder, headerBasePath)
                sourcePath = os.path.join(baseFolder, baseName + ".cpp")

                if baseFolder and not os.path.exists(baseFolder):
                    os.makedirs(baseFolder, exist_ok=True)

                headerPathBackup = MakeBackup(headerPath)
                with open(headerPath, mode="w+", encoding="utf8") as headerHandle:
                    headerHandle.write("#pragma once\n\n")
                    headerHandle.write("\n".join(GenerateHeaderBody(node)) + "\n")
                CleanBackupIfUnchanged(headerPath, headerPathBackup)

                sourcePathBackup = MakeBackup(sourcePath)
                with open(sourcePath, mode="w+", encoding="utf8") as sourceHandle:
                    sourceHandle.write(f"#include \"{headerBasePath}\"\n") 
                    sourceHandle.write("#include \"tools.h\"\n") 
                    sourceHandle.write("#include <math.h>\n") 
                    sourceHandle.write("#include <algorithm>\n\n") 
                    sourceHandle.write(formatter("\n".join(GenerateSourceBody(node))) + "\n")

                CleanBackupIfUnchanged(sourcePath, sourcePathBackup)

                cxxRootNode.SopAdd(node, f"#include \"{headerPath}\"\n")
                cxxRootNode.SopDelete(node)                

    newOriginData = cxxRootNode.GetSourceOperationsResults(compileCheck=False)

    for file, data in newOriginData.items():
        fileBackup = MakeBackup(file)
        openMode = "w+"
        if testDrive:
            openMode = "r"
        with open(file, mode=openMode, encoding="utf8") as originHandle:
            originHandle.seek(0, os.SEEK_SET)
            if not testDrive:
                originHandle.truncate()
                originHandle.write(data)
            if len(baseFolder) == 0:
                baseFolder = os.path.splitext(os.path.basename(file))[0]
        CleanBackupIfUnchanged(file, fileBackup)

def MakeBackup(filePath):
    if os.path.exists(filePath):
        dirName = os.path.dirname(filePath)
        baseName = os.path.basename(filePath)
        backupGlob = os.path.join(dirName, "backup.*."+baseName)
        highest = 0
        for used in glob.glob(backupGlob):
            usedName = os.path.basename(used)
            num = int(usedName.split(".")[1])
            if num > highest:
                highest = num
        backupPath = os.path.join(dirName, f"backup.{highest+1}."+baseName)
        shutil.copy(filePath, backupPath)
        return backupPath
    return None

def CleanBackupIfUnchanged(filePath, backupFilePath):
    if backupFilePath and filecmp.cmp(filePath, backupFilePath, shallow=False):
        os.unlink(backupFilePath)

def GenerateHeaderBody(node, indentLevel = 0):
    from ..cppmodel.CxxClass import CxxClass
    from ..cppmodel.CxxFunction import CxxFunction
    from ..cppmodel.CxxVariable import CxxVariable
    from ..cppmodel.CxxTemplate import CxxTemplate

    output = []

    indentation = OPT_INDENTION_CHAR * (indentLevel * OPT_INDENTION_LEVEL)
    indentationInner = OPT_INDENTION_CHAR * (indentLevel * OPT_INDENTION_LEVEL + OPT_INDENTION_LEVEL)

    if type(node) is CxxClass:
        if node.sourceObject.kind is cindex.CursorKind.UNION_DECL:
            output.append(f"""{indentation}union {node}""")
            output.append(f"""{indentation}{{""")
            output = output + GenerateHeader(node, indentLevel + 1)
            output.append(f"""{indentation}}};""")

        if node.sourceObject.kind is cindex.CursorKind.STRUCT_DECL:
            output.append(f"""{indentation}struct {node}""")
            output.append(f"""{indentation}{{""")
            output = output + GenerateHeader(node, indentLevel + 1)
            output.append(f"""{indentation}}};""")

        if node.sourceObject.kind is cindex.CursorKind.CLASS_DECL:
            output.append(f"""{indentation}class {node}""")
            output.append(f"""{indentation}{{""")
            output = output + GenerateHeader(node, indentLevel + 1)
            output.append(f"""{indentation}}};""")

    if type(node) in [CxxFunction, CxxVariable, CxxTemplate]:
        output.append(f"""{indentation}{node};""")

    return output

def GenerateHeader(cxxRootNode, indentLevel = 0):
    output = []

    for node in cxxRootNode.forEachChild(noDepth = True):
        output = output + GenerateHeaderBody(node, indentLevel)

    return output

def GenerateSource(cxxRootNode, context = [], indentLevel = 0):
    output = []

    for node in cxxRootNode.forEachChild(noDepth = True):
        output = output + GenerateSourceBody(node, context, indentLevel)

    return output

def GenerateSourceBody(node, context = [], indentLevel = -1):
    from ..cppmodel.CxxClass import CxxClass
    from ..cppmodel.CxxFunction import CxxFunction
    from ..cppmodel.CxxVariable import CxxVariable

    output = []

    indentation = OPT_INDENTION_CHAR * (indentLevel * OPT_INDENTION_LEVEL)
    indentationInner = OPT_INDENTION_CHAR * (indentLevel * OPT_INDENTION_LEVEL + OPT_INDENTION_LEVEL)

    if type(node) is CxxClass:
        if node.sourceObject.kind is cindex.CursorKind.UNION_DECL:
            pass

        if node.sourceObject.kind is cindex.CursorKind.STRUCT_DECL or node.sourceObject.kind is cindex.CursorKind.CLASS_DECL:
            context.append(f"{node}")
            output = output + GenerateSource(node, context, indentLevel + 1)
            context.pop()
  
    if type(node) is CxxFunction:
        args = node.cxxArgumentTypeNameList()
        parsedArgs = []
        for arg in args:
            parsedArgs.append(arg.type.spelling + " " + arg.spelling)

        body = node.cxxBody()

        constDecl = ""
        if node.sourceObject.type.is_const_qualified() or node.isConst:
            constDecl = " const"

        memberInitializerList = ""
        if node.has_init_list:
            tokenList = list(map(lambda t: t.spelling, list(node.sourceObject.get_tokens())))
            startIndex = tokenList.index(":")
            endIndex = tokenList.index("{")
            memberInitializerList = ":" + " ".join(tokenList[startIndex + 1 : endIndex])

        output.append(f"""{indentation}{node.cxxReturnType()} {"::".join(node.cxxNamespaceList() + context + [node.symbolName()])}({", ".join(parsedArgs)}){constDecl}
{indentation}{memberInitializerList}{body}
{indentation}""")

    if type(node) is CxxVariable:
        if node.sourceObject.storage_class == cindex.StorageClass.STATIC:
            symbolDecl = "::".join(context+[node.symbolName()])
            output.append(f"{indentation}{node.cxxType()} {symbolDecl};")

    return output

def LoadExtent(node):
    startFilePath = str(node.first_child.extent.start.file)
    endFilePath = str(node.sourceObject.extent.end.file)
    if not startFilePath == endFilePath:
        raise RuntimeError(f"Unable to load Extent: start and end sourcelocations not same path: {[str(startFilePath), str(endFilePath)]}")
    if not os.path.exists(startFilePath):
        raise RuntimeError(f"Unable to load Extent: file does not exists: {str(startFilePath)}")

    startIndex = node.first_child.extent.start.offset + 1
    endIndex = node.sourceObject.extent.end.offset
    size = endIndex - startIndex - 1
    with open(startFilePath) as handle:
        handle.seek(startIndex)
        source = handle.read(size)

    return source

def GenerateFunctionTemplateValues(cxxFunc):
    cppArgumentTypes = GetFunctionArgumentsTypes(cxxFunc)

    cppOriginalFunctionName = cxxFunc.sourceObject.spelling
    cppReturnType = cxxFunc.sourceObject.result_type.spelling
    cppNamespaceHierarchy = cxxFunc.getContainingNamespaces(cxxFunc.sourceObject, [])
    documentComment = json.dumps(str(cxxFunc.comment()))

    return {
        "cppArgumentTypes": cppArgumentTypes,
        "cppOriginalFunctionName": cppOriginalFunctionName,
        "cppReturnType": cppReturnType,
        "cppNamespaceHierarchy": cppNamespaceHierarchy,
        "documentComment": documentComment        
    }

def GetFunctionArgumentsTypes(cxxFunc):
    cppargs = []
    for arg in cxxFunc.forEachArgument():
        cppargs.append(arg.spelling)

    return cppargs