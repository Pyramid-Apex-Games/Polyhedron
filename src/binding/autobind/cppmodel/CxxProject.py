import os
import json
from ..symbol_db import SymbolsByFilename

class CxxProject():
    def __init__(self, parser, buildFolder, rootFolder):
        self.parser = parser
        self.buildFolder = buildFolder
        self.rootFolder = rootFolder
        self.projectData = {}
        self.compileDb = []

    def InitializeCompileDb(self):
        compileDbFile = os.path.join(self.buildFolder, "compile_commands.json")
        parentPath = os.path.dirname(self.buildFolder)

        if not os.path.exists(compileDbFile):
            raise RuntimeError(f"Error: file compile_commands.json not found in {self.buildFolder}")

        with open(compileDbFile, "r") as compileDbHandle:
            self.compileDb = json.load(compileDbHandle)

    def Initialize(self):
        self.InitializeCompileDb()

        files = [cc['file'] for cc in self.compileDb]

        for file in files:
            self.InitializeFile(file)

        # import pdb; pdb.set_trace()
        # print (f"Project---\n{json.dumps(self.projectData, indent=4, sort_keys=True)}")

    def GenerateRefukkerTemplate(self, outputFolder, withCameltoer = False):
        os.makedirs(outputFolder, exist_ok=True)
        templateFile = os.path.join(outputFolder, "refukker.json")

        templateData = {}

        for file, data in self.projectData.items():
            templateItemData = {
                "from": {
                    "symbol": data['Name']
                },
                "to": {
                    "file": file,
                    "name": data['Name']
                },
                "options": {
                }
            }
            if withCameltoer and 'CamelCase' in data and data['CamelCase']:
                templateItemData['to']['name'] = data['CamelCase']

        # with open(templateFile) as templateFileHandle:
        #     pass

    def ListFilesInDirectory(self, directory):
        output = []
        for file in os.listdir(directory):
            fullPath = os.path.join(directory, file)
            if os.path.isdir(fullPath):
                output = output + self.ListFilesInDirectory(fullPath)
            else:
                if file.endswith(".cpp") or file.endswith(".h"):
                    output.append(fullPath)
        
        return output
    
    def InitializeFile(self, file):
        if not 'files' in self.projectData:
            self.projectData['files'] = {}

        self.projectData['files'][file] = {
            'symbols': SymbolsByFilename(file, self.buildFolder)
        }

