""" #LLDB (and probably GDB) Visualizers for debugging bliss
"""

bootcommand = """type synthetic add vec --python-class PolyhedronVisualizers.Vec_SyntheticChildrenProvider
type synthetic add ivec --python-class PolyhedronVisualizers.Vec_SyntheticChildrenProvider
type synthetic add usvec --python-class PolyhedronVisualizers.Vec_SyntheticChildrenProvider
type synthetic add svec --python-class PolyhedronVisualizers.Vec_SyntheticChildrenProvider
type synthetic add bvec --python-class PolyhedronVisualizers.Vec_SyntheticChildrenProvider
type synthetic add hvec --python-class PolyhedronVisualizers.Vec_SyntheticChildrenProvider
type synthetic add vec2 --python-class PolyhedronVisualizers.Vec2_SyntheticChildrenProvider
type synthetic add ivec2 --python-class PolyhedronVisualizers.Vec2_SyntheticChildrenProvider
type synthetic add hvec2 --python-class PolyhedronVisualizers.Vec2_SyntheticChildrenProvider
type synthetic add vec4 --python-class PolyhedronVisualizers.Vec4_SyntheticChildrenProvider
type synthetic add ivec4 --python-class PolyhedronVisualizers.Vec4_SyntheticChildrenProvider
type synthetic add hvec4 --python-class PolyhedronVisualizers.Vec4_SyntheticChildrenProvider
type synthetic add dvec4 --python-class PolyhedronVisualizers.Vec4_SyntheticChildrenProvider
type synthetic add squat --python-class PolyhedronVisualizers.Vec4_SyntheticChildrenProvider
type summary add -F PolyhedronVisualizers.Vector_SyntheticChildrenProvider -e -x "^vector<[^>]+>$" -w PolyhedronVisualizers
type synthetic add -x "^vector<[^>]+>$" --python-class PolyhedronVisualizers.Vector_SyntheticChildrenProvider
"""

import lldb
import lldb.formatters.Logger

def __lldb_init_module(debugger, dict):
    logger = lldb.formatters.Logger.Logger()
    logger >> "__lldb_init_module"

    # lldb.formatters.Logger.__lldb_formatters_debug_level = 2
    # lldb.formatters.Logger.__lldb_formatters_debug_filename = "/Users/micha/dev/cube/Polyhedron/.lldb.log"

    for command in bootcommand.split("\n"):
        if command:
            debugger.HandleCommand(command)

class Vec_SyntheticChildrenProvider():
    def __init__(self, valobj, internal_dict):
        # Initialize the Python object using valobj as the variable to provide synthetic children.
        self.sbValObject = valobj

    def num_children(self):
        # Return the number of children you want your object to have.
        return 3

    def get_child_index(self,name):
        # Return the index of the synthetic child whose name is given as argument.
        if name == "x":
            return 0
        elif name == "y":
            return 1
        elif name == "z":
            return 2

        return None
        
    def get_child_at_index(self,index):
        # Return a new LLDB SBValue object representing the child at the index given as argument.
        if index == 0:
            return self.sbValObject.GetChildMemberWithName("x")
        elif index == 1:
            return self.sbValObject.GetChildMemberWithName("y")
        elif index == 2:
            return self.sbValObject.GetChildMemberWithName("z")
        else:
            return None

    def update(self):
        # Update the internal state of this Python object whenever the state of the variables in LLDB changes.
        pass

    def has_children(self):
        # Return True if this object might have children and False if this object can be guaranteed to not have children.
        return True

    def get_value(self):
        # Return an SBValue to be presented as the value of the synthetic value under consideration.
        return self.sbValObject


class Vec2_SyntheticChildrenProvider():
    def __init__(self, valobj, internal_dict):
        # Initialize the Python object using valobj as the variable to provide synthetic children.
        self.sbValObject = valobj

    def num_children(self):
        # Return the number of children you want your object to have.
        return 2

    def get_child_index(self,name):
        # Return the index of the synthetic child whose name is given as argument.
        if name == "x":
            return 0
        elif name == "y":
            return 1

        return None
        
    def get_child_at_index(self,index):
        # Return a new LLDB SBValue object representing the child at the index given as argument.
        if index == 0:
            return self.sbValObject.GetChildMemberWithName("x")
        elif index == 1:
            return self.sbValObject.GetChildMemberWithName("y")
        else:
            return None

    def update(self):
        # Update the internal state of this Python object whenever the state of the variables in LLDB changes.
        pass

    def has_children(self):
        # Return True if this object might have children and False if this object can be guaranteed to not have children.
        return True

    def get_value(self):
        # Return an SBValue to be presented as the value of the synthetic value under consideration.
        return self.sbValObject


class Vec4_SyntheticChildrenProvider():
    def __init__(self, valobj, internal_dict):
        # Initialize the Python object using valobj as the variable to provide synthetic children.
        self.sbValObject = valobj

    def num_children(self):
        # Return the number of children you want your object to have.
        return 4

    def get_child_index(self,name):
        # Return the index of the synthetic child whose name is given as argument.
        if name == "x":
            return 0
        elif name == "y":
            return 1
        elif name == "z":
            return 2
        elif name == "w":
            return 3

        return None
        
    def get_child_at_index(self,index):
        # Return a new LLDB SBValue object representing the child at the index given as argument.
        if index == 0:
            return self.sbValObject.GetChildMemberWithName("x")
        elif index == 1:
            return self.sbValObject.GetChildMemberWithName("y")
        elif index == 2:
            return self.sbValObject.GetChildMemberWithName("z")
        elif index == 3:
            return self.sbValObject.GetChildMemberWithName("w")
        else:
            return None

    def update(self):
        # Update the internal state of this Python object whenever the state of the variables in LLDB changes.
        pass

    def has_children(self):
        # Return True if this object might have children and False if this object can be guaranteed to not have children.
        return True

    def get_value(self):
        # Return an SBValue to be presented as the value of the synthetic value under consideration.
        return self.sbValObject

class Cube_SyntheticChildrenProvider():
    def __init__(self, valObject, internal_dict):
        self.sbValObject = valObject

    def num_children(self):
        # Return the number of children you want your object to have.
        return 9

    def get_child_index(self,name):
        # Return the index of the synthetic child whose name is given as argument.
        if name == "children":
            return 0
        elif name == "ext":
            return 1
        elif name == "edges":
            return 2
        elif name == "faces":
            return 3
        elif name == "texture":
            return 4
        elif name == "material":
            return 5
        elif name == "merged":
            return 6
        elif name == "escaped":
            return 7
        elif name == "visible":
            return 8

        return None
        
    def get_child_at_index(self,index):
        namemap = [
            "children",
            "ext",
            "edges",
            "faces",
            "teture",
            "material",
            "escaped",
            "visible"
        ]
        if index >= 0 and index < len(namemap):
            return self.sbValObject.GetChildMemberWithName(namemap[index])

        return None

    def update(self):
        # Update the internal state of this Python object whenever the state of the variables in LLDB changes.
        pass

    def has_children(self):
        # Return True if this object might have children and False if this object can be guaranteed to not have children.
        return True

    def get_value(self):
        # Return an SBValue to be presented as the value of the synthetic value under consideration.
        return self.sbValObject


class Vector_SyntheticChildrenProvider():
    def __init__(self, valObject, internal_dict):
        logger = lldb.formatters.Logger.Logger()
        logger >> "ctor"
        self.sbValObject = valObject
        self.template_arg = valObject.GetType().GetTemplateArgumentType(0)
        self.update()
        

    def num_children(self):
        logger = lldb.formatters.Logger.Logger()
        logger >> "num_children"
        # Return the number of children you want your object to have.
        return self.ulen.GetValueAsUnsigned()

    def get_child_index(self, name):
        logger = lldb.formatters.Logger.Logger()
        logger >> "get_child_index"
        # Return the index of the synthetic child whose name is given as argument.
        try:
            return int(name.lstrip('[').restrip(']'))
        except:
            pass
        
    def get_child_at_index(self, index):
        logger = lldb.formatters.Logger.Logger()
        logger >> "get_child_at_index"
        if index >= 0 and index < self.ulen.GetValueAsUnsigned():
            try:
                offset = self.data_size * index
                return self.buf.CreateChildAtOffset("[" + str(index) + "]", offset, self.data_type)
            except:
                pass

        return None

    def update(self):
        logger = lldb.formatters.Logger.Logger()
        logger >> "update"
        # Update the internal state of this Python object whenever the state of the variables in LLDB changes.
        self.buf = self.sbValObject.GetChildMemberWithName("buf")
        self.data_type = self.buf.GetType().GetPointeeType()
        self.data_size = self.buf.GetType().GetByteSize()
        self.alen = self.sbValObject.GetChildMemberWithName("alen")
        self.ulen = self.sbValObject.GetChildMemberWithName("ulen")
        pass

    def has_children(self):
        logger = lldb.formatters.Logger.Logger()
        logger >> "has_children"
        # Return True if this object might have children and False if this object can be guaranteed to not have children.
        return True

    def get_value(self):
        logger = lldb.formatters.Logger.Logger()
        logger >> "get_value"
        # Return an SBValue to be presented as the value of the synthetic value under consideration.
        return self.sbValObject