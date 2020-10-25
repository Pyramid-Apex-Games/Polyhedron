from .CxxNode import CxxNode
import os
from clang import cindex
import copy
import sys
import tempfile

class CxxTranslationUnit(CxxNode):

    def __init__(self, parser, sourceObject, parent = None):
        from clang import cindex

        CxxNode.__init__(self, parser, sourceObject.cursor, parent)
        self.sourceOps = []
        self.translation_unit = sourceObject

    def __str__(self):
        sourceLoc = self.sourceObject.extent.start.file.name
        if sourceLoc:
            return sourceLoc
        return "tu-no-file"

    def OptimizeRanges(self, rangeList, dropList = []):
        for i in range(0, len(rangeList)):
            if i in dropList:
                continue
            i_val = rangeList[i]
            i_hs = i_val['size'] * 0.5
            i_c = i_val['start'] + i_hs

            for j in range(i + 1, len(rangeList)):
                j_val = rangeList[j]
                j_hs = j_val['size'] * 0.5
                j_c = j_val['start'] + j_hs

                if abs(i_c - j_c) < i_hs+j_hs:
                    # Overlap, merge ranges
                    if i_val['start'] > j_val['start']:
                        i_val['start'] = j_val['start']
                    if i_val['end'] < j_val['end']:
                        i_val['end'] = j_val['end']
                    newSize = i_val['end'] - i_val['start']
                    if newSize != i_val['size']:
                        i_val['size'] = newSize
                        i_hs = i_val['size'] * 0.5
                        i_c = i_val['start'] + i_hs
                    dropList.append(j)
        
        for d in dropList:
            rangeList[d] = None
        for i in range(len(rangeList) - 1, -1, -1):
            if rangeList[i] == None:
                before = rangeList[0:i]
                after = rangeList[i+1:]
                rangeList = before + after

        return rangeList

    def GetSourceRanges(self, cxxRootNode, ranges = {}):
        self.GetSourceRange(cxxRootNode, ranges)

        for node in cxxRootNode.forEachChild(noDepth = False):
            self.GetSourceRange(node, ranges)

        for path, rangeList in ranges.items():
            ranges[path] = self.OptimizeRanges(rangeList)

        return ranges

    def GetSourceRange(self, node, exportRanges):
        if node.sourceObject.extent:
            filePath = str(node.sourceObject.extent.start.file)
            if not filePath in exportRanges:
                exportRanges[filePath] = []
            exportRanges[filePath].append({
                'start': node.sourceObject.extent.start.offset,
                'end': node.sourceObject.extent.end.offset,
                'size': node.sourceObject.extent.end.offset - node.sourceObject.extent.start.offset
            })

    SourceOp_Delete = 1
    SourceOp_AddBefore = 2
    SourceOp_AddAfter = 3

    SourceOpStrMap = {
        1: "Delete",
        2: "AddBefore",
        2: "AddAfter"
    }

    def AppendSourceOperation(self, op, nodes, __context = None):
        for node in nodes:
            if __context is None:
                context = {}
            else:
                context = copy.deepcopy(__context)

            if not 'file' in context:
                context['file'] = node.sourceObject.location.file.name
            if not 'ranges' in context:
                context['ranges'] = self.GetSourceRanges(node)

            self.sourceOps.append({'op': op, 'nodes': nodes, 'context': context})

    def SopDelete(self, node):
        self.AppendSourceOperation(CxxTranslationUnit.SourceOp_Delete, [node])

    def SopAdd(self, node, data, beforeCursor = True):
        if beforeCursor:
            self.AppendSourceOperation(CxxTranslationUnit.SourceOp_AddBefore, [node], {'data': data})
        else:
            self.AppendSourceOperation(CxxTranslationUnit.SourceOp_AddAfter, [node], {'data': data})

    def GetSourceOperationsResults(self, filePath = None, compileCheck = True):
        output = ""
        skippedOutput = []
        if filePath is None:
            filePath = str(self)

        otherFiles = []

        with open(filePath) as handle:
            handle.seek(0, os.SEEK_END)
            endPos = handle.tell()
            handle.seek(0, os.SEEK_SET)
            for op in self.sourceOps:
                for file, ranges in op['context']['ranges'].items():
                    if file == filePath:
                        for r in ranges:
                            curPos = handle.tell()
                            output = output + handle.read(r['start'] - curPos)
                            if op['op'] == CxxTranslationUnit.SourceOp_Delete:
                                skippedOutput.append(handle.read(r['size']))
                            elif op['op'] == CxxTranslationUnit.SourceOp_AddBefore:
                                output = output + op['context']['data']
                            elif op['op'] == CxxTranslationUnit.SourceOp_AddAfter:
                                output = output + handle.read(r['size'])
                                output = output + op['context']['data']
                    else:
                        otherFiles.append(file)

            curPos = handle.tell()
            output = output + handle.read(endPos - curPos)
        
        if compileCheck:
            from ..compileflags import CompileFlagsFor

            os.chdir(os.path.dirname(filePath))
            index = cindex.Index.create()
            flags = [
                "-fparse-all-comments",
            ] + CompileFlagsFor(filePath, "../../build")

            success = False
            try:
                translation_unit = index.parse(filePath, flags, unsaved_files=[(filePath, output)])
                if translation_unit.diagnostics:
                    fatalErrorList = []
                    for diag in translation_unit.diagnostics:
                        # print (diag, file=sys.stderr)
                        if diag.severity > cindex.Diagnostic.Warning:
                            fatalErrorList.append(diag)
                    if fatalErrorList:
                        tmpDescriptor, tmpFileName = tempfile.mkstemp(os.path.basename(filePath))
                        with open(tmpDescriptor, "w+") as tmpHandle:
                            writtenByteCount = tmpHandle.write(output)
                            print(f"{tmpFileName}: wrote {writtenByteCount} bytes")

                        for i in range(0, len(fatalErrorList)):
                            fatalErrorList[i] = str(fatalErrorList[i]).replace(filePath, tmpFileName)
                        errorStr = "\n".join(fatalErrorList)
                        raise cindex.TranslationUnitLoadError(f"Error parsing:\n{errorStr}")
                success = True
            except cindex.TranslationUnitLoadError as e:
                print (f"Error: {filePath}: {e}", file=sys.stderr)

            if not success:
                raise RuntimeError(f"GetSourceOperationsResults: Applying the source operations to file {filePath} results in an invalid cpp file!")

        otherOutput = {}
        for file in otherFiles:
            otherOutput = {**otherOutput, **self.GetSourceOperationsResults(file, compileCheck=compileCheck)}

        otherOutput[filePath] = output

        return otherOutput

