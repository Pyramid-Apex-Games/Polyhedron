from .CxxNode import CxxNode

class CxxInclude(CxxNode):
    def __init__(self, parser, sourceObject, absoluteFile, parent = None):
        CxxNode.__init__(self, parser, sourceObject, parent)
        self.absoluteFile = absoluteFile
        pass
