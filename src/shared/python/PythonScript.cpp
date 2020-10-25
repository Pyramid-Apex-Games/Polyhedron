#include "shared/python/PythonScript.h"
#include "engine/main/Application.h"
#include "PythonPipeIO.h"
#define PY_SSIZE_T_CLEAN
#include <Python.h>

namespace {
    wchar_t* s_ProgramNamePtr;
    wchar_t* s_PythonHomeDir = L"config/site-py";
}

PythonScript::PythonScript(const std::string& programName, const std::string& pythonHome)
{
    PyImport_AppendInittab("polyio", PyInit_PythonPipeIO);
    Py_NoSiteFlag = 1;
    Py_FrozenFlag = 1;
    Py_IgnoreEnvironmentFlag = 1;
    Py_NoUserSiteDirectory = 1;
    Py_OptimizeFlag = 1;
//    Py_UnbufferedStdioFlag = 1;

    if (!pythonHome.empty())
    {
        s_PythonHomeDir = Py_DecodeLocale(pythonHome.c_str(), NULL);
    }

    Py_SetPythonHome(s_PythonHomeDir);
    Py_SetPath(s_PythonHomeDir);

    s_ProgramNamePtr = Py_DecodeLocale(programName.c_str(), NULL);
    if (s_ProgramNamePtr == nullptr)
    {
        Application::Instance().Fatal("Python: Fatal error: cannot decode " + programName);
    }
    Py_SetProgramName(s_ProgramNamePtr);
    Py_Initialize();
    PyImport_ImportModule("polyio");

    conoutf("Python: %s", Py_GetVersion());
    Update();
}

PythonScript::~PythonScript()
{
    if (Py_FinalizeEx() < 0)
    {
        Application::Instance().Fatal("Python: Finalization Failed");
    }

    PyMem_RawFree(s_ProgramNamePtr);
}

void PythonScript::RunString(const std::string& code)
{
    PyRun_SimpleString(code.c_str());
    Update();
}

void PythonScript::RunFile(const std::string& file)
{
    std::string buf = loadfile(file.c_str(), NULL);
    RunString(buf);
}

void PythonScript::Update()
{

}