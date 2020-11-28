import json
import os
import json
import sys

cached_flags = None

def LoadFlags(buildpath):
    global cached_flags
    storeFile = os.path.abspath(os.path.join(buildpath, "compile_commands.json"))
    if not os.path.exists(storeFile):
        raise RuntimeError(f"Missing compile_commands.json in {buildpath}! Generate one by using the Unix Makefile or the Ninja generator.")
    if cached_flags is None:
        with open(storeFile, "r") as file:
            cached_flags = json.load(file)
    return cached_flags

def CompileFlagsFor(file, buildFolder):
    global cached_flags
    if cached_flags is None:
        LoadFlags(buildFolder)

    fileabs = os.path.abspath(file)
    for tu in cached_flags:
        tuFile = os.path.normpath(tu['file'])
        tuDir = os.path.normpath(tu['directory'])
        if tuFile == fileabs:
            command = list(filter(None, tu['command'].split()))
            flags = []
            for c in command:
                if c[:1] == "-":
                    flags.append(c)
                elif c[:1] == "@" and c[-4:] == ".rsp":
                    #gcc type include weirdness detected
                    rspFilePath = os.path.join(tuDir, c[1:])
                    with open(rspFilePath) as rspHandle:
                        rspFileContents = rspHandle.read().split(" ")
                        for flag in rspFileContents:
                            if flag[:1] == "-":
                                flags.append(flag)
                else:
                    if flags:
                        c = flags.pop() + " " + c
                        flags.append(c) 
            return NormalizeCompileFlags(buildFolder, flags)

    fileabsstr = str(fileabs)
    if fileabsstr.endswith(".h"):
        return ["-x", "c++-header"] + CompileFlagsFor(fileabsstr[:-2] + ".cpp", buildFolder)


    raise RuntimeError(f"Missing entry for {file} in compile_commands.json in {buildFolder}! Perhaps you have to regenerate the compile_commands.json?")


def NormalizeCompileFlags(buildFolder, flags):
    oldworkdir = os.getcwd()
    os.chdir(buildFolder)
    output = []
    for flag in flags:
        if flag[:2] == "-I":
            importPath = os.path.abspath(flag[2:])
            output.append("-I"+importPath)
        elif flag[:2] == "-o":
            pass
        elif flag[:2] == "-c":
            pass
        else:
            if ' ' in flag:
                output = output + flag.split()
            else:
                output.append(flag)
    os.chdir(oldworkdir)
    # if '-DANDROID' in output:
    #     output.remove("-DANDROID")
    return output
