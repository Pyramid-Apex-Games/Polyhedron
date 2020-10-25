
import clang.cindex

clang.cindex.Config.set_library_file("/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/libclang.dylib")

index = clang.cindex.Index.create()

source = """
template <typename T> class X {};
X<bool> x;

template <typename T> func(T t) {};
"""

######### stolen from cymbal

from ctypes import c_uint, c_int

def find_libclang_function(function):
    return getattr(clang.cindex.conf.lib, function)

def monkeypatch_helper(classtype, name, library_function, args, result):
    if hasattr(classtype, name):
        raise ('failed to add method, %s is already available' % name) 
    f = find_libclang_function(library_function)
    f.argtypes = args
    f.restype = result
    def impl(*args):
        return f(*args)
    setattr(classtype, name, impl)

def monkeypatch_type(method_name, library_function, args, result):
    monkeypatch_helper(clang.cindex.Type, method_name, library_function, args, result)



monkeypatch_type('get_template_argument_type',
                        'clang_Type_getTemplateArgumentAsType',
                        [clang.cindex.Type, c_uint],
                        clang.cindex.Type)

monkeypatch_type('get_num_template_arguments',
                        'clang_Type_getNumTemplateArguments',
                        [clang.cindex.Type],
                        c_int)
######### /stolen from cymbal

# helpers for visiting the AST recursively
def visit(node, func):
    func(node)
    for c in node.get_children():
        visit(c, func)

def visit_depth(node, func, depth=0):
    func(node, depth)
    for c in node.get_children():
        visit_depth(c, func, depth+1)

# parse the TU
tu = clang.cindex.TranslationUnit.from_source('t.cpp', ['-std=c++11'], unsaved_files=[('t.cpp', source)])

# show the AST
def astprinter(node, depth):
    print (" "*depth, node.kind, node.spelling)
visit_depth(tu.cursor, astprinter)    

# find everything with a template and try to extract the template argument
def template_finder(node):
    if hasattr(node, 'type') and node.type.get_num_template_arguments() != -1:
        print (node.type.get_num_template_arguments(), node.spelling, node.kind, node.get_template_argument_type(0).kind)
visit(tu.cursor, template_finder)