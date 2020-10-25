from .CxxNode import CxxNode

class PhuiElement(CxxNode):

    def __init__(self, parser, sourceObject, parent, data):
        CxxNode.__init__(self, parser, sourceObject, parent)
        self.data = data
        print(f"~~~ PhuiElement {data}")

    def __str__(self):
        return "PhUI: " + str(self.data)