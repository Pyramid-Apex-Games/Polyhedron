import json
import subprocess
import os
import sys


def Execute(command, arguments, environment, workdir):
    print(command, arguments)
    env = os.environ.copy()
    env.update(environment)
    with subprocess.Popen([command] + arguments, env=env, cwd=workdir) as proc:
        print(proc.stdout.read())

def BuildModelJsonToCMakeCommand(buildModelJsonPath):
    with open(buildModelJsonPath) as buildModelHandle:
        buildModel = json.load(buildModelHandle)

    env = {}
    buildDir = ""
    sourceDir = ""
    command = ""
    arguments = []

    if 'buildSettings' in buildModel:
        if 'environmentVariables' in buildModel['buildSettings']:
            pass

    if 'cmake' in buildModel:
        if 'effectiveConfiguration' in buildModel['cmake']:
            if 'buildRoot' in buildModel['cmake']['effectiveConfiguration']:
                buildDir = os.path.realpath(buildModel['cmake']['effectiveConfiguration']['buildRoot'])
            if 'cmakeExecutable' in buildModel['cmake']['effectiveConfiguration']:
                command = os.path.realpath(buildModel['cmake']['effectiveConfiguration']['cmakeExecutable'])
            if 'variables' in buildModel['cmake']['effectiveConfiguration']:
                for keyVarPair in buildModel['cmake']['effectiveConfiguration']['variables']:
                    arguments.append("-D" + keyVarPair["name"] + "=" + keyVarPair["value"])
    if 'variant' in buildModel:
        if 'module' in buildModel['variant']:
            cmakeListsFile = buildModel['variant']['module']['makeFile']
            sourceDir = os.path.dirname(os.path.realpath(cmakeListsFile))

    arguments.append("-H" + sourceDir)
    arguments.append("-B" + buildDir)
    Execute(command, arguments, env, buildDir)

if __name__ == "__main__":
    BuildModelJsonToCMakeCommand(sys.argv[1])