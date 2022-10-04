#pragma once
#include <map>
#include <vector>
#include "Locker.h"
#include "singleton.h"
#include <string>
#include "grus_impl.h"
#include "utility.h"
#include "jitlib.h"

enum class JitType
{
    func,
    object
};
class JITManager :
    public Singleton<JITManager>
{
public:
    JITManager();
    ~JITManager();
    void Init();

    PyObject* Register(PyObject* args, PyObject* kwargs);
    PyObject* CreateClassWrapper(PyObject* args, PyObject* kwargs);
    PyObject* CreateFuncWrapper(PyObject* args, PyObject* kwargs);
    PyObject* Generate(PyObject* args, PyObject* kwargs);
    PyObject* Generate_UID(PyObject* args, PyObject* kwargs);
    void Init(PyObject* args, PyObject* kwargs);
    void SetPath(std::string s)
    {
        mModuleRootFolder = s;
    }
    std::string& GetPath()
    {
        return mModuleRootFolder;
    }
    std::string GetCompilerPath()
    {
        return mCompiler_Path;
    }
    void SetThisModule(PyObject* m)
    {
        mThisModule = m;
    }
    PyObject* GetThisModule()
    {
        return mThisModule;
    }
    void* QueryOrCreateClassObject(void* selfofcaller,
        const char* class_name, void* pNativeObj);
    JitClassInfo* FindClassFromNameSpace(std::string nm);
    PyObject* FindClassProxyType(std::string className);
private:
    PyObject* mThisModule = nullptr;
    bool SearchVC_Compiler(std::string& vc_init_file);
    bool SearchVC_CompilerWithPath(std::string root,std::string& vc_init_file);

    void ParseKWArgs(JitLib* pLib,LangType langType,JitFuncInfo* funcInfo, PyObject* kwargs);
    void ParsePyModuleInfo(PyObject* kwargs,std::string& strModuleFileName, 
        std::string& strLangType, std::string& strLibName);
    std::string mCompiler_Path;
    std::string mModuleRootFolder;
    JitLib* GetOrCreateLib(bool addModule,std::string callerPyFile, std::string strLibName,int* pModuleIndex =nullptr);
    std::map<std::string, JitLib*> mJitlibs;
    JitLib* SearchLibBySharedLibName(std::string name);
};