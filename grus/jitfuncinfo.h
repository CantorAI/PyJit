#pragma once

#include <map>
#include <vector>
#include <string>
#include "grus_impl.h"
#include "Jit_Object.h"
#include "jitlib.h"

struct VarInfo
{
	std::string name;
	std::string type;
	std::string defaultValue;
	void* getter = nullptr;
	void* setter = nullptr;
	std::string bindto;//binding information to class
};
struct ClassFuncInfo
{
	std::string name;
	std::string returnType;
	std::vector<VarInfo> parameters;
	void* stubfunc = nullptr;
};

class JitFuncInfo
{
	friend class JitLib;

	const char* implKey = "impl";
public:
	JitFuncInfo();
	~JitFuncInfo();

	void AddCfg(std::string key, PyJit::Object val);
	LangType SetLang(std::string& strType);
	void SetCode(std::string& strCode);
	virtual void SetName(std::string& name);
	inline void SetLib(JitLib* pLib)
	{
		mJitLib = pLib;
	}
	JitLib* Lib()
	{
		return mJitLib;
	}
	inline void SetModuleIndex(int idx)
	{
		m_moduleIndex = idx;
	}
	PyObject* Call(PyObject* args, PyObject* kwargs);
	bool IsExternImpl();
	std::vector<std::string> GetExternImplFileName();
	std::vector<std::string> GetIncludesFileName();
	std::string& Hash() { return m_hash; }
	std::string& Name() { return m_name; }
	std::string& Code() { return m_code; }
	ClassFuncInfo& FuncHead() { return m_funcHead; }
	void SetFuncHead(ClassFuncInfo& funcInfo)
	{
		m_funcHead = funcInfo;
	}
	virtual void SetHash(std::string& h)
	{
		m_hash = h;
	}
	int ModuleIndex() { return m_moduleIndex; }
	LangType Lang() { return m_lang; }
	std::map<std::string, PyJit::Object>& GetCfg()
	{
		return m_cfg;
	}
	void ParseFuncInfo_use_annotations(std::string& name,
		PyJit::Object& funcObject,
		ClassFuncInfo& funcInfo,
		std::string& strfuncDesc);
	bool ParseFuncInfo(std::string& code,
		ClassFuncInfo& funcInfo,
		std::string& strfuncBody);
	inline void AddInclude(std::string& incFile)
	{
		mIncludeFiles.push_back(incFile);
	}
	inline std::vector<std::string>& IncludeFiles()
	{
		return mIncludeFiles;
	}
	void BuildRelativeFileSpec(std::string& spec);
	void SetClassName(std::string& className)
	{
		m_className = className;
	}
	std::string& ClassName()
	{
		return m_className;
	}
protected:
	bool ParseBindInfo(std::string strInfo, VarInfo& varInfo);
	std::string MakeFileChangeTimeSpec(std::string filename);

	std::vector<std::string> mIncludeFiles;//keep them to check if these files changed
	ClassFuncInfo m_funcHead;
	JitLib* mJitLib = nullptr;
	std::map<std::string, PyJit::Object> m_cfg;
	int m_moduleIndex = 0;
	std::string m_className;//use to change Python class name to native(c++)class name include namespace as prefix
	std::string m_name;//function name
	std::string m_code;//function while body before translating to C++
	std::string m_hash;//store code's md5 hash
	LangType m_lang;
};
