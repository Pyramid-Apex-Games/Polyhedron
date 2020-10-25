import subprocess
import json

CLANG_FORMAT_BINARY = "clang-format"
ENCODING = "utf-8"

def formatter(inputString, assumedFilename = "src/somefile.cpp", style = "Google"):
    command = [
        CLANG_FORMAT_BINARY,
        "-assume-filename", assumedFilename,
        "-style", style,
        # "-lines", "all"
    ]

    p = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdin=subprocess.PIPE
    )
    stdout, stderr = p.communicate(input=inputString.encode(ENCODING))

    if stderr:
        raise ClangFormatRuntimeError(stderr)

    if not stdout:
        raise ClangFormatRuntimeError(
            'No output from clang-format (crashed?).\n'
            'Please report to bugs.llvm.org.'
        )
    
    lines = stdout.decode(ENCODING).split('\n')

    return "\n".join(lines)


class ClangFormatRuntimeError(RuntimeError):
    def __init__(self, error):
        RuntimeError.__init__(self, error)