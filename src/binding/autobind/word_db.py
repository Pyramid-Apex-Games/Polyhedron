import os
import json
import re

cached_wordDatabase = {}
WORD_DB_PATH = [
    "words/words_alpha.txt",
    "src/binding/autobind/words/words_alpha.txt",
]
CAPTURE_ALPHANUMERIC = re.compile('[^a-zA-Z0-9_ ]+', re.UNICODE)
CAPTURE_DIGITS = re.compile('(\d+)')

def HasWord(wordDatabaseFile, word):
    global cached_wordDatabase
    if not wordDatabaseFile in cached_wordDatabase:
        LoadWordDatabase(wordDatabaseFile)
    if word[0:2] in cached_wordDatabase[wordDatabaseFile]["words"]:
        for wordElement in cached_wordDatabase[wordDatabaseFile]["words"][word[0:2]]:
            if word == wordElement:
                return True
    return False

def LoadWordDatabase(wordDatabaseFile):
    global cached_wordDatabase
    cached_wordDatabase[wordDatabaseFile] = {
        "index": [],
        "words": {}
    }

    if not os.path.exists(wordDatabaseFile + ".index"):
        index, words = CreateWordDatabase(wordDatabaseFile)
        cached_wordDatabase[wordDatabaseFile]["index"] = cached_wordDatabase[wordDatabaseFile]["index"] + index
        cached_wordDatabase[wordDatabaseFile]["words"].update(words)
    else:
        with open(wordDatabaseFile + ".index") as indexFileHandle:
            loadedIndex = json.load(indexFileHandle)
            cached_wordDatabase[wordDatabaseFile]["index"] = cached_wordDatabase[wordDatabaseFile]["index"] + loadedIndex["index"]
            cached_wordDatabase[wordDatabaseFile]["words"].update(loadedIndex["words"])

def CreateWordDatabase(wordDatabaseFile):
    a_chr = ord("a")
    print(f"cwd: {os.getcwd()}")

    with open(wordDatabaseFile) as wordsHandle:
        index = []
        words = {}
        byteIndex = 0
        curBlock = 0
        line = ""
        check = False
        while True:
            byte = wordsHandle.read(1)
            byteIndex += 1
            print(f"\rIndexing {wordDatabaseFile} {byteIndex}",end="")
            if not byte:
                break
            if (byte == "\n"):
                check = True
            else:
                line += byte
            if check and len(line) > 1:
                lineBlockId0 = ord(line[0:1]) - a_chr
                lineBlockId1 = ord(line[1:2]) - a_chr
                lineBlockId = lineBlockId0 * 26 + lineBlockId1
                byteVal = byteIndex - len(line)
                while len(index) - curBlock > 0:
                    index.append(byteVal)
                if not line[0:2] in words:
                    words[line[0:2]] = []
                words[line[0:2]].append(line)
                check = False
                line = ""
                curBlock = lineBlockId
        with open(wordDatabaseFile + ".index", "w+") as indexHandle:
            json.dump({"index": index, "words": words}, indexHandle, indent=4)
            indexHandle.truncate()
        return index, words

def TokenizeGreedy(wordDatabaseFile, word):
    if '_' in word:
        return word.split("_")
    if word[0:1].isupper():
        return [word]
    if re.match(CAPTURE_ALPHANUMERIC, word):
        return CAPTURE_ALPHANUMERIC.sub('', word).split(" ")
    if re.search(CAPTURE_DIGITS, word):
        return re.split(CAPTURE_DIGITS, word)

    word = word.lower()

    startPtr = 0
    endPtr = len(word)

    parts = []

    while True:
        for start in range(startPtr, endPtr):
            found = False
            for end in range(endPtr, start, -1):
                subWord = word[start:end]
                hasWord = HasWord(wordDatabaseFile, subWord)
                # print (f"{start}:{end}: {subWord} ({len(parts)}) {hasWord}")
                # width = (end - start)
                if hasWord or subWord.isdigit():
                    parts.append(subWord)
                    start = start + len(subWord)
                    found = True
                    break
            if found:
                startPtr = start
                break
        if start > len(word) or startPtr == endPtr or len(parts) == 0:
            break

    if len(parts) > 0:
        if len(word) > startPtr:
            parts.append(word[startPtr:])
    else:
        print (f"Warning: could not match any word in {word}")
        parts.append(word)

    return parts

def TokenizeComplete(wordDatabaseFile, word):
    if '_' in word:
        return word.split("_")
    if word[0:1].isupper():
        return [word]
    if re.match(CAPTURE_ALPHANUMERIC, word):
        pWord = CAPTURE_ALPHANUMERIC.sub('', word)
        return pWord.split(" ")
    if re.search(CAPTURE_DIGITS, word):
        pWord = re.split(CAPTURE_DIGITS, word)
        if pWord:
            pWord.pop()
            return pWord

    word = word.lower()

    startPtr = 0
    endPtr = len(word)

    found_db = []

    for start in range(startPtr, endPtr):
        for end in range(endPtr, start, -1):
            subWord = word[start:end]
            hasWord = HasWord(wordDatabaseFile, subWord)
            if hasWord:
                found_db.append([start, end, subWord])

    found_db_len = len(found_db)
    result = []

    TokenizeCompleteProcessDb(found_db, found_db_len, result, 0, endPtr)
    return list(map(lambda e: e[2], result))


def TokenizeCompleteProcessDb(db, db_len, context, start, word_len):
    for itr in range(start, db_len):
        if len(context) == 0 or db[itr][0] == context[-1][1]:
            context.append(db[itr])
            popAfter = True
        else:
            popAfter = False
            
        if context[-1][1] == word_len:
            return True

        if TokenizeCompleteProcessDbRecurse(db, db_len, context, itr + 1, word_len):
            return True
        else:
            if popAfter:
                context.pop()
    return False

def TokenizeCompleteProcessDbRecurse(db, db_len, context, start, word_len):
    if len(context) > 0:
        for i in range(start, db_len):
            if db[i][0] == context[-1][1]:
                return TokenizeCompleteProcessDb(db, db_len, context, i, word_len)
    
    return False

def ToCamelCase(word):
    global WORD_DB_PATH
    for word_db in WORD_DB_PATH:
        if os.path.exists(word_db):
            return "".join(map(lambda subWord: subWord.capitalize(), TokenizeComplete(word_db, word)))


testwords = [
    "sethomedir",
    "uni2cubechars",
    "GetUiNt",
    "putfloat_",
    "randomMT",
    "tmpidx",
    "setattrib",
    "numattribs",
    "vbo",
    "cubectype",
    "packagedirs",
    "ziparchive",
    "cos360",
    "attachentities",
    "<deduction for type array>",
    "half",
    "htcmp",
    "dualquat"
]

for test in testwords:
    print(f"{test} => {ToCamelCase(test)}")