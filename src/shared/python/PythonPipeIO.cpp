#include "PythonPipeIO.h"
#include "engine/console.h"
#include "engine/main/Application.h"
#include <string>

struct PythonPipeIO
{
    PyObject_HEAD
};
struct PythonPipeIOErr
{
    PyObject_HEAD
};

PyObject* PythonPipeIO_write(PyObject* self, PyObject* args)
{
    std::size_t written(0);
    PythonPipeIO* selfimpl = reinterpret_cast<PythonPipeIO*>(self);
    char* data;
    if (!PyArg_ParseTuple(args, "s", &data))
        return 0;

    written = strlen(data);
    if (written > 0)
    {
        std::string buffer(data, written);
        Application::Instance().GetConsole().AddPiecewise(CON_INFO, buffer);
    }

    return PyLong_FromSize_t(written);
}

PyObject* PythonPipeIO_write_err(PyObject* self, PyObject* args)
{
    std::size_t written(0);
    PythonPipeIOErr* selfimpl = reinterpret_cast<PythonPipeIOErr*>(self);
    char* data;
    if (!PyArg_ParseTuple(args, "s", &data))
        return 0;

    written = strlen(data);
    if (written > 0)
    {
        std::string buffer(data, written);
        Application::Instance().GetConsole().AddPiecewise(CON_ERROR, buffer);
    }

    return PyLong_FromSize_t(written);
}

PyObject* PythonPipeIO_flush(PyObject* self, PyObject* args)
{
    std::string buffer = "\n";
    Application::Instance().GetConsole().AddPiecewise(buffer);
    return Py_BuildValue("");
}

PyObject* PythonPipeIO_flush_err(PyObject* self, PyObject* args)
{
    std::string buffer = "\n";
    Application::Instance().GetConsole().AddPiecewise(CON_ERROR, buffer);
    return Py_BuildValue("");
}

PyMethodDef PythonPipeIO_methods[] =
{
    {"write", PythonPipeIO_write, METH_VARARGS, "sys.stdout.write"},
    {"flush", PythonPipeIO_flush, METH_VARARGS, "sys.stdout.flush"},
    {0, 0, 0, 0} // sentinel
};


PyMethodDef PythonPipeIOErr_methods[] =
{
    {"write", PythonPipeIO_write_err, METH_VARARGS, "sys.stderr.write"},
    {"flush", PythonPipeIO_flush_err, METH_VARARGS, "sys.stderr.flush"},
    {0, 0, 0, 0} // sentinel
};

PyTypeObject PythonPipeIOType =
{
    PyVarObject_HEAD_INIT(0, 0)
    "polyio.PythonPipeIO",/* tp_name */
    sizeof(PythonPipeIO), /* tp_basicsize */
    0,                    /* tp_itemsize */
    0,                    /* tp_dealloc */
    0,                    /* tp_print */
    0,                    /* tp_getattr */
    0,                    /* tp_setattr */
    0,                    /* tp_reserved */
    0,                    /* tp_repr */
    0,                    /* tp_as_number */
    0,                    /* tp_as_sequence */
    0,                    /* tp_as_mapping */
    0,                    /* tp_hash  */
    0,                    /* tp_call */
    0,                    /* tp_str */
    0,                    /* tp_getattro */
    0,                    /* tp_setattro */
    0,                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,   /* tp_flags */
    "polyhedron.PythonPipeIO object", /* tp_doc */
    0,                    /* tp_traverse */
    0,                    /* tp_clear */
    0,                    /* tp_richcompare */
    0,                    /* tp_weaklistoffset */
    0,                    /* tp_iter */
    0,                    /* tp_iternext */
    PythonPipeIO_methods, /* tp_methods */
    0,                    /* tp_members */
    0,                    /* tp_getset */
    0,                    /* tp_base */
    0,                    /* tp_dict */
    0,                    /* tp_descr_get */
    0,                    /* tp_descr_set */
    0,                    /* tp_dictoffset */
    0,                    /* tp_init */
    0,                    /* tp_alloc */
    PyType_GenericNew,    /* tp_new */
};

PyTypeObject PythonPipeIOErrType =
{
    PyVarObject_HEAD_INIT(0, 0)
    "polyio.PythonPipeIOErr",/* tp_name */
    sizeof(PythonPipeIO), /* tp_basicsize */
    0,                    /* tp_itemsize */
    0,                    /* tp_dealloc */
    0,                    /* tp_print */
    0,                    /* tp_getattr */
    0,                    /* tp_setattr */
    0,                    /* tp_reserved */
    0,                    /* tp_repr */
    0,                    /* tp_as_number */
    0,                    /* tp_as_sequence */
    0,                    /* tp_as_mapping */
    0,                    /* tp_hash  */
    0,                    /* tp_call */
    0,                    /* tp_str */
    0,                    /* tp_getattro */
    0,                    /* tp_setattro */
    0,                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,   /* tp_flags */
    "polyhedron.PythonPipeIOErr object", /* tp_doc */
    0,                    /* tp_traverse */
    0,                    /* tp_clear */
    0,                    /* tp_richcompare */
    0,                    /* tp_weaklistoffset */
    0,                    /* tp_iter */
    0,                    /* tp_iternext */
    PythonPipeIOErr_methods, /* tp_methods */
    0,                    /* tp_members */
    0,                    /* tp_getset */
    0,                    /* tp_base */
    0,                    /* tp_dict */
    0,                    /* tp_descr_get */
    0,                    /* tp_descr_set */
    0,                    /* tp_dictoffset */
    0,                    /* tp_init */
    0,                    /* tp_alloc */
    PyType_GenericNew,    /* tp_new */
};


PyModuleDef PythonPipeIOModule =
{
    PyModuleDef_HEAD_INIT,
    "polyio", 0, -1, 0,
};

PyObject* s_sys_stdout;
PyObject* s_sys_stderr;

PyMODINIT_FUNC PyInit_PythonPipeIO(void)
{
    s_sys_stdout = nullptr;
    s_sys_stderr = nullptr;

    if (PyType_Ready(&PythonPipeIOType) < 0)
        return 0;

    if (PyType_Ready(&PythonPipeIOErrType) < 0)
        return 0;

    PyObject* m = PyModule_Create(&PythonPipeIOModule);
    if (m)
    {
        Py_INCREF(&PythonPipeIOModule);
        PyModule_AddObject(m, "polyio", reinterpret_cast<PyObject*>(&PythonPipeIOType));

        s_sys_stdout = PythonPipeIOType.tp_new(&PythonPipeIOType, 0, 0);
        PySys_SetObject("stdout", s_sys_stdout);

        s_sys_stderr = PythonPipeIOErrType.tp_new(&PythonPipeIOErrType, 0, 0);
        PySys_SetObject("stderr", s_sys_stderr);
    }
    return m;
}