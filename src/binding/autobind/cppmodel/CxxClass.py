from .CxxNode import CxxNode
from clang import cindex

class CxxClass(CxxNode):
    
    def __init__(self, parser, sourceObject, parent = None):
        CxxNode.__init__(self, parser, sourceObject, parent)

    def NamespaceBlock(self, body):
        namespaces = self.getContainingNamespaces(self.sourceObject, [])
        if namespaces:
            output = body
            for ns in namespaces:
                output = "namespace {} {{\n{}\n}}".format(ns, output)
            return output
        else:
            return body

    def __str__(self):
        className = self.sourceObject.spelling
        namespaces = self.getContainingNamespaces(self.sourceObject, [])
        for ns in namespaces:
            className = "{}::{}".format(ns, className)
        return className

    def GenerateForwardDeclaration(self):
        if self.sourceObject.kind is cindex.CursorKind.UNION_DECL:
            decl  = f"union {self};"

        elif self.sourceObject.kind is cindex.CursorKind.STRUCT_DECL:
            decl  = f"struct {self};"

        elif self.sourceObject.kind is cindex.CursorKind.CLASS_DECL:
            decl  = f"class {self};"

        else:
            raise RuntimeError(f"CxxClass: GenerateForwardDeclaration: TypeKind not implemented: {node.sourceObject.kind}")

        return self.NamespaceBlock(decl)
