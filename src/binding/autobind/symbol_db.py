import yaml
import os
import json
import tempfile
import subprocess
import sys
import shutil
from multiprocessing import cpu_count
from threading import Thread
from queue import Queue

from .word_db import ToCamelCase

cached_symbol_db = None

CLANG_FIND_ALL_SYMBOLS_BINARY = "/Users/micha/dev/clang-dev/llvm/build/RelWithDebInfo/bin/find-all-symbols"
ENCODING = "utf-8"

def ExecCommand(threadId, *command):
    print(f"{threadId}> Start {' '.join(command)}")
    p = subprocess.Popen(
        command,
        # shell=True,
        # stdout=subprocess.PIPE,
        # stderr=subprocess.PIPE,
        # stdin=subprocess.PIPE
    )
    p.wait()
    print(f"{threadId}> Finished {' '.join(command)}")


def ExecCommandThread(threadId, queue):
    print(f"Start thread {threadId}")
    while True:
        command = queue.get()
        ExecCommand(threadId, *command)
        queue.task_done()
    print(f"End thread {threadId}")


def CreateSymbolDb(buildpath):
    compileDbFile = os.path.join(buildpath, "compile_commands.json")
    parentPath = os.path.dirname(buildpath)

    if not os.path.exists(compileDbFile):
        raise RuntimeError(f"Error: file compile_commands.json not found in {buildpath}")

    with open(compileDbFile, "r") as compileDbHandle:
        compileDb = json.load(compileDbHandle)


    cached_symbol_db = {
        "all": [],
        "byName": {},
        "byFile": {},
        "allExternal": [],
        "byExternalName": {},
        "byExternalFile": {},
    }
    docCount = 0
    fileCount = 0
    symbolCount = 0

    tmpdir = tempfile.mkdtemp()

    commands = [
        [
            CLANG_FIND_ALL_SYMBOLS_BINARY, 
            entry['file'], 
            "-output-dir=" + tmpdir, 
            "-p=" + buildpath
        ] for entry in compileDb
    ]

    poolSize = cpu_count() - 1
    print(f"Parsing {len(commands)} files in {poolSize} threads ..")

    queue = Queue()
    threadPool = [Thread(target=ExecCommandThread, args=(threadId, queue)) for threadId in range(poolSize)]
    for thread in threadPool:
        thread.daemon = True
        thread.start()
    
    for command in commands:
        queue.put(command)

    queue.join()

    print("done, merging")

    tmpSymbolYaml = tempfile.mktemp()
    mergeCommand = [CLANG_FIND_ALL_SYMBOLS_BINARY, "-merge-dir=" + tmpdir, tmpSymbolYaml]

    mergeProcess = subprocess.Popen(
        mergeCommand,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdin=subprocess.PIPE
    )
    stdout, stderr = mergeProcess.communicate()

    with open(tmpSymbolYaml, "r") as symbolFileHandle:
        symbolCount += 1
        curDoc = {}
        curKey = ""
        for event in yaml.parse(symbolFileHandle):
            # print (f"event {event} ", end="")
            if type(event) == yaml.DocumentStartEvent:
                curDoc = {}

            if type(event) == yaml.DocumentEndEvent:
                curDoc['CamelCase'] = curDoc['Name']
                if not (curDoc['FilePath'].startswith("<") and curDoc['FilePath'].endswith(">")):
                    curDoc['FilePath'] = os.path.abspath(curDoc['FilePath'])

                if curDoc['FilePath'].startswith(parentPath) and not curDoc['FilePath'].startswith(buildpath):
                    if curDoc['Type'] in ['Function', 'Variable']:
                        curDoc['CamelCase'] = ToCamelCase(curDoc['Name'])

                    cached_symbol_db['all'].append(curDoc)
                    if curDoc['Name'] in cached_symbol_db['byName']:
                        if not type(cached_symbol_db['byName'][curDoc['Name']]) == list:
                            cached_symbol_db['byName'][curDoc['Name']] = [cached_symbol_db['byName'][curDoc['Name']]]
                        cached_symbol_db['byName'][curDoc['Name']].append(curDoc)
                    else:
                        cached_symbol_db['byName'][curDoc['Name']] = curDoc

                    if not curDoc['FilePath'] in cached_symbol_db['byFile']:
                        cached_symbol_db['byFile'][curDoc['FilePath']] = []
                    cached_symbol_db['byFile'][curDoc['FilePath']].append(curDoc)
                else:
                    cached_symbol_db['allExternal'].append(curDoc)
                    if curDoc['Name'] in cached_symbol_db['byExternalName']:
                        if not type(cached_symbol_db['byExternalName'][curDoc['Name']]) == list:
                            cached_symbol_db['byExternalName'][curDoc['Name']] = [cached_symbol_db['byExternalName'][curDoc['Name']]]
                        cached_symbol_db['byExternalName'][curDoc['Name']].append(curDoc)
                    else:
                        cached_symbol_db['byExternalName'][curDoc['Name']] = curDoc

                    if not curDoc['FilePath'] in cached_symbol_db['byExternalFile']:
                        cached_symbol_db['byExternalFile'][curDoc['FilePath']] = []
                    cached_symbol_db['byExternalFile'][curDoc['FilePath']].append(curDoc)

                print (f"\r\033[Kfiles: {fileCount} symbol dbs: {symbolCount} docs: {docCount}: {curDoc['Name']}", end="")
                curDoc = {}

                docCount += 1

            if type(event) == yaml.ScalarEvent:
                if curKey == "":
                    curKey = event.value
                else:
                    if curKey in curDoc and type(curDoc[curKey]) == list:
                        curDoc[curKey].append(event.value)
                    else:
                        curDoc[curKey] = event.value
                        curKey = ""

            if type(event) == yaml.SequenceStartEvent:
                if curKey != "":
                    if not curKey in curDoc:
                        curDoc[curKey] = []

            if type(event) == yaml.SequenceEndEvent:
                curKey = ""
    print ("")
    # os.unlink(tmpSymbolYaml)
    # shutil.rmtree(tmpdir)

    return cached_symbol_db

def LoadSymbolDb(buildpath):
    global cached_symbol_db
    databaseCacheFile = os.path.abspath(os.path.join(buildpath, "find_all_symbols_db.json"))

    if cached_symbol_db is None:
        if os.path.exists(databaseCacheFile):
            with open(databaseCacheFile, "r") as cacheHandle:
                cached_symbol_db = json.load(cacheHandle)
        else:
            cached_symbol_db = CreateSymbolDb(buildpath)

            print (f"writing {databaseCacheFile}")
            with open(databaseCacheFile, "w+") as cacheHandle:
                cacheHandle.seek(0)
                json.dump(cached_symbol_db, cacheHandle, indent=4)
                cacheHandle.truncate()

    return cached_symbol_db

def FilenameBySymbol(symbolName, buildpath):
    symbol_db = LoadSymbolDb(buildpath)

    for symbol_def in symbol_db:
        if symbol_def['Name'] == symbolName:
            return symbol_def['FilePath']

def SymbolsByFilename(fileName, buildpath):
    symbol_db = LoadSymbolDb(buildpath)

    if fileName in symbol_db['byFile']:
        return symbol_db['byFile'][fileName]

    return []


class ClangFindAllSymbolsRuntimeError(RuntimeError):
    def __init__(self, error):
        RuntimeError.__init__(self, error)