#include "grus.h"
#include "grus_impl.h"
#include "jitfunc.h"
#include "jitclassproxy.h"
#include "jitmgr.h"
#include "Jit_Object.h"

#if (!WIN32)
#include <dlfcn.h>
#endif

extern "C"
{
#include "numpy/arrayobject.h"
}


#ifdef PythonType2
PyMODINIT_FUNC initGrus(void)
{
    PyObject* m;
    Py_Initialize();
    PyImport_AddModule("grus");
    Py_InitModule("grus", RootMethods);
}
#else
static PyModuleDef GrusPackageTypeModule =
{
    PyModuleDef_HEAD_INIT,
    "grus",
    "grus Objects",
    -1,
    RootMethods, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC PyInit_grus(void)
{
#if (WIN32)
    //HMODULE  hModule = GetModuleHandle("grus.pyd");
    HMODULE  hModule = NULL;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCTSTR)PyInit_grus,
        &hModule);
    char path[MAX_PATH];
    GetModuleFileName(hModule, path, MAX_PATH);
    std::string strPath(path);
    auto pos = strPath.rfind("\\");
    if (pos != std::string::npos)
    {
        strPath = strPath.substr(0, pos+1);
    }
    strPath += "\\pyjit";
#else
    Dl_info dl_info;
    dladdr((void*)PyInit_grus, &dl_info);
    std::string strPath = dl_info.dli_fname;
    auto pos = strPath.rfind("/");
    if (pos != std::string::npos)
    {
        strPath = strPath.substr(0, pos + 1);
    }
    strPath += "/pyjit";
#endif
    JITManager::I().SetPath(strPath);

    JITManager::I().Init();
    PyObject* m = PyModule_Create(&GrusPackageTypeModule);
    JITManager::I().SetThisModule(m);
    PyObject* gObj = PyDict_New();
    if (PyModule_AddObject(m, "g", gObj) < 0)
    {
        Py_DECREF(gObj);
    }
    return m;
}
#endif

