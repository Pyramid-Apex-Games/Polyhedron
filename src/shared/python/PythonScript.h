#pragma once
#ifdef BUILD_WITH_PYTHON
#include <string>

class PythonScript {
public:
    PythonScript(const std::string& programName, const std::string& pythonHome = "");
    ~PythonScript();

    void RunString(const std::string& code);
    void RunFile(const std::string& file);

    void Update();
private:
};
#endif
