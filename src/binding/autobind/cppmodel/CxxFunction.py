from .CxxNode import CxxNode, Generator
import json
import os
from clang import cindex
from .. import parsecpp
from ..generator import CubeScriptBinding

class CxxFunction(CxxNode):
    def __init__(self, parser, sourceObject, parent = None, generateFor = Generator.All):
        CxxNode.__init__(self, parser, sourceObject, parent, generateFor)
        self.customFuctionName = None
        self.first_child = next(sourceObject.get_children(), None)
        for self.first_child in sourceObject.get_children():
            if self.first_child.kind == cindex.CursorKind.ANNOTATE_ATTR and self.first_child.spelling.startswith(parsecpp.EXPORT_ANNOTATION):
                start = self.first_child.spelling.rindex(parsecpp.EXPORT_ANNOTATION)
                if start == 0:
                    self.customFuctionName = self.first_child.spelling[len(parsecpp.EXPORT_ANNOTATION):]
                self.first_child = next(sourceObject.get_children(), None)
            elif self.first_child.kind == cindex.CursorKind.COMPOUND_STMT:
                break
            else:
                self.first_child = next(sourceObject.get_children(), None)

        tokenList = list(self.sourceObject.get_tokens())
        tokenListStr = list(map(lambda x: x.spelling, tokenList))

        self.is_explicit = False
        self.is_inline = False
        self.has_init_list = False

        if self.sourceObject.kind == cindex.CursorKind.CONSTRUCTOR:
            if ':' in tokenListStr:
                self.has_init_list = True

        if len(tokenList) > 0:
            firstToken = tokenList[0]
            if firstToken.kind == cindex.TokenKind.KEYWORD:
                if firstToken.spelling == "explicit":
                    self.is_explicit = True
                if firstToken.spelling == "inline":
                    self.is_inline = True

    def forEachArgumentNameType(self):
        def iterate(node):
            if node.type.kind.name == "FUNCTIONPROTO":
                for c in node.get_arguments():
                    yield c

        yield from iterate(self.sourceObject)

    def forEachArgument(self):
        def iterate(node):
            if node.type.kind.name == "FUNCTIONPROTO":
                for c in node.type.argument_types():
                    yield c
            # for c in node.get_children():
            #     yield from iterate(c)
        
        yield from iterate(self.sourceObject)

    def comment(self):
        if self.sourceObject.brief_comment is None:
            return ""
        return self.sourceObject.brief_comment

    def LoadExtent(self):
        startFilePath = str(self.sourceObject.extent.start.file)
        endFilePath = str(self.sourceObject.extent.end.file)
        if not startFilePath == endFilePath:
            raise RuntimeError(f"Unable to load Extent: start and end sourcelocations not same path: {[str(startFilePath), str(endFilePath)]}")
        if not os.path.exists(startFilePath):
            raise RuntimeError(f"Unable to load Extent: file does not exists: {str(startFilePath)}")

        startIndex = self.sourceObject.extent.start.offset
        endIndex = self.sourceObject.extent.end.offset
        size = endIndex - startIndex
        with open(startFilePath) as handle:
            handle.seek(startIndex)
            source = handle.read(size)

        return source

    def __str__(self):
        args = []
        for arg in self.cxxArgumentList():
            args.append(arg.spelling)
        functionName = self.symbolName()
        namespace = self.cxxNamespace()
        if namespace:
            functionName = namespace + "::" +functionName
        returntype = self.cxxReturnType() + " "
        if self.sourceObject.kind in [cindex.CursorKind.CONSTRUCTOR,
                                   cindex.CursorKind.DESTRUCTOR]:
            returntype = ""
        explicitQual = ""
        if self.is_explicit:
            explicitQual = "explicit "
        inlineToken = ""
        if self.is_inline:
            inlineToken = "inline "
        constDecl = ""
        if self.sourceObject.type.is_const_qualified() or self.isConst:
            constDecl = " const"

        return inlineToken + explicitQual + returntype + functionName + "(" + (", ".join(args)) + ")" + constDecl

    def cxxArgumentList(self):
        args = []
        for arg in self.forEachArgument():
            args.append(arg)
        return args
    
    def cxxArgumentTypeNameList(self):
        args = []
        for arg in self.forEachArgumentNameType():
            args.append(arg)
        return args        

    def cxxReturnType(self):
        if self.sourceObject.kind in [cindex.CursorKind.CONSTRUCTOR,
                                   cindex.CursorKind.DESTRUCTOR]:
            return ""        
        return self.sourceObject.result_type.spelling

    def cxxNamespaceList(self):
        return self.getContainingNamespaces(self.sourceObject, [])

    def cxxNamespace(self):
        return "::".join(self.cxxNamespaceList())

    def cxxBody(self):
        if not self.sourceObject.is_definition():
            workSourceObject = self.sourceObject.get_definition()
        else:
            workSourceObject = self.sourceObject

        bodyCursor = None
        for child in workSourceObject.get_children():
            if child.kind == cindex.CursorKind.COMPOUND_STMT:
                bodyCursor = child
                break

        if not bodyCursor:
            raise RuntimeError(f"Unable to locate function body of {self}!")

        return " ".join(list(map(lambda t: t.spelling, bodyCursor.get_tokens())))

    def symbolName(self):
        functionName = self.sourceObject.spelling
        if self.customFuctionName:
            functionName = self.customFuctionName
        return functionName