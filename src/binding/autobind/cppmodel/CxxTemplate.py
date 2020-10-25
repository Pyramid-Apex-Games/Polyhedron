from clang import cindex
from .CxxFunction import CxxFunction

class CxxTemplate(CxxFunction):
    def __init__(self, parser, sourceObject, parent = None):
        CxxFunction.__init__(self, parser, sourceObject, parent)
        self.underlyingCursorKind = cindex.CursorKind.from_id(cindex.conf.lib.clang_getTemplateCursorKind(sourceObject))
        def visit(node, parentOf, appendTo):
            #if node.lexical_parent == parentOf:
            if node.kind in [cindex.CursorKind.TEMPLATE_TYPE_PARAMETER, 
                            cindex.CursorKind.TEMPLATE_NON_TYPE_PARAMETER,
                            cindex.CursorKind.TEMPLATE_TEMPLATE_PARAMETER]:
                appendTo.append(node)
            for c in node.get_children():
                visit(c, parentOf, appendTo)
        self.templateArgs = []
        visit(self.sourceObject, self.sourceObject, self.templateArgs)

    def forEachTemplateArgument(self):
        for arg in self.templateArgs:
            if arg.kind == cindex.CursorKind.TEMPLATE_TYPE_PARAMETER:
                yield "typename " + arg.spelling
            else:
                yield "/*FIXME " + str(arg.kind) + "*/ typename " + arg.spelling

    def forEachTemplateArgumentBlah(self):

        ######### from cymbal
        from ctypes import c_uint, c_int

        def find_libclang_function(function):
            return getattr(cindex.conf.lib, function)

        def monkeypatch_helper(classtype, name, library_function, args, result):
            if hasattr(classtype, name):
                #raise Exception(f"failed to add method, {name} is already available") 
                return
            f = find_libclang_function(library_function)
            f.argtypes = args
            f.restype = result
            def impl(*args):
                return f(*args)
            setattr(classtype, name, impl)

        def monkeypatch_type(method_name, library_function, args, result):
            monkeypatch_helper(cindex.Type, method_name, library_function, args, result)

        monkeypatch_type('get_template_argument_type',
                                'clang_Type_getTemplateArgumentAsType',
                                [cindex.Type, c_uint],
                                cindex.Type)

        monkeypatch_type('get_num_template_arguments',
                                'clang_Type_getNumTemplateArguments',
                                [cindex.Type],
                                c_int)
        ######### /from cymbal

        for argIndex in range(0, self.sourceObject.get_num_template_arguments()):
            yield self.sourceObject.get_template_argument_type(argIndex).spelling

    def __str__(self):
        templateArgs = ", ".join(list(self.forEachTemplateArgument()))
        return f"template<{templateArgs}> " + CxxFunction.__str__(self)
