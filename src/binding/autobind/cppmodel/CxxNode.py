from clang import cindex
from enum import Flag, auto
import sys

class Generator(Flag):
    Noop = 0
    Json = auto()
    Attributes = auto()
    CubeScript = auto()
    Python = auto()

    All = Json | Attributes | CubeScript | Python

class CxxNode():
    
    def __init__(self, parser, sourceObject, parent = None, generateFor = Generator.Noop):
        # self.cxxtype = cxxtype
        self.sourceObject = sourceObject
        self.parent = parent
        self.children = []
        self.generateFor = generateFor

        self.getQualifiers()

        if not self.parent is None:
            self.parent.addchild(self)
        
    def getQualifiers(self):
        self.isConst = 0
        self.isVolatile = 0
        self.isRestrict = 0
        if hasattr(self.sourceObject, 'get_usr'):
            usr = self.sourceObject.get_usr()
            bangLoc = usr.rfind("#")
            if bangLoc >= 0 and len(usr) > bangLoc+1:
                bangValue = usr[bangLoc+1:]
                try:
                    qual = int(bangValue)
                    self.isConst = qual & 0x1
                    self.isVolatile = qual & 0x4
                    self.isRestrict = qual & 0x2
                except ValueError:
                    pass

    def handle_phui_field(self, parser, annotation_cursor):
        from .PhuiElement import PhuiElement

        annotation_data = annotation_cursor.spelling.split(";")
        PhuiElement(parser, annotation_cursor, self, annotation_data)

    def addchild(self, node):
        node.parent = self
        if node not in self.children:
            self.children.append(node)
        pass

    def delchild(self, node):
        if node in self.children:
            self.children.remove(node)
            node.parent = None
            return True
        return False
    
    def forEachChild(self, depthFirst = False, noDepth = False):
        for child in self.children:
            if not noDepth and depthFirst:
                yield from child.forEachChild(depthFirst)
            yield child
            if not noDepth and not depthFirst:
                yield from child.forEachChild(depthFirst)

    def dump(self, level = 0):
        pretext = " |" * level
        output = pretext + " +"+ self.__class__.__qualname__ + " " + str(self) + "\n"
        for child in self.children:
            output += child.dump(level + 1)

        return output


    def getContainingNamespaces(self, node, data):
        if node is None:
            data = self.getContainingNamespaces(self.sourceObject, data)
        else:
            if node:
                if (node.kind == cindex.CursorKind.NAMESPACE):
                    data.append(node.spelling)
                if node.semantic_parent and node.semantic_parent.kind != cindex.CursorKind.TRANSLATION_UNIT:
                    data = self.getContainingNamespaces(node.semantic_parent, data)
        return data

    def __str__(self):
        return self.sourceObject.spelling