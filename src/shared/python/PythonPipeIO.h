#ifdef BUILD_WITH_PYTHON
#define PY_SSIZE_T_CLEAN
#include <Python.h>

PyMODINIT_FUNC PyInit_PythonPipeIO(void);
#endif