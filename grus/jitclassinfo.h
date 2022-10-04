#pragma once

#include "jitfuncinfo.h"
#include "jitclassproxy.h"


class JitClassInfo :
	public JitFuncInfo
{
public:
	JitClassInfo();
	~JitClassInfo();
	void Reset();
	void BuildClassInfo(PyObject* obFunc, std::string& className,std::string& fingerprints);

	bool CheckNeedBuild()
	{
		return !m_IsClassStubLoaded;
	}
	void CopyNecessaryPropsFromWrapperFuncInfo(JitFuncInfo* pWrapperFuncInfo);
	void SetStubs(int propNum, int methodNum, 
		std::vector<std::string>& classMemberNames,
		std::vector<unsigned long long>& stubs);
	void SetStoredHash(const char* hash);
	virtual void SetHash(std::string& h);

	std::vector<VarInfo>& Props()
	{
		return m_props;
	}
	std::vector<ClassFuncInfo>& Funcs()
	{
		return m_classfuncs;
	}
	ClassFuncInfo& InitFuncInfo()
	{
		return m_initfuncInfo;
	}
	bool HaveInitFunc()
	{
		return m_have_init_func;
	}
	virtual void SetName(std::string& name) override;
	const std::string& InitFuncName()
	{
		return m_init_func_name;
	}
	inline void* NewStub() { return m_newstub; }
	inline void* DeallocStub() { return m_deallocstub; }
	inline void* SerializStub() { return m_serialize_stub; }
	inline void setTypeMethodSetGet_Assigned(bool b)
	{
		m_isTypeMethodSetGet_Assigned = b;
	}
	inline bool TypeMethodSetGet_Assigned()
	{
		return m_isTypeMethodSetGet_Assigned;
	}
	void AddInstance(void* nativeObj, PyJitClassProxy* proxy)
	{
		m_instanceMap.emplace(std::make_pair(nativeObj, proxy));
	}
	void RemoveInstance(void* nativeObj)
	{
		m_instanceMap.erase(nativeObj);
	}
	PyJitClassProxy* QueryInstance(void* nativeObj)
	{
		PyJitClassProxy* proxy = nullptr;
		auto it = m_instanceMap.find(nativeObj);
		if (it != m_instanceMap.end())
		{
			proxy = it->second;
		}
		return proxy;
	}
	std::string NativeClassName()
	{
		return m_className.size()==0?m_name: m_className;
	}
	inline bool support_serialization()
	{
		return m_support_serialization;
	}
	std::string GetFullNameSpace();
	void SetJitClassType(JitClassType* p)
	{
		m_pJitClassType = p;
	}
	JitClassType* GetJitClassType()
	{
		return m_pJitClassType;
	}
protected:
	std::string m_HashStored;//stored into code,and compile into Shared Lib, loaded from code
	//need to compare with hash calacuted from class code
	bool m_IsClassStubLoaded = false;
	bool m_isTypeMethodSetGet_Assigned = false;

	bool m_support_serialization = false;
	const std::string m_init_func_name = "__init__";
	std::vector<VarInfo> m_props;
	std::vector<ClassFuncInfo> m_classfuncs;
	ClassFuncInfo m_initfuncInfo;
	bool m_have_init_func = false;
	void* m_newstub = nullptr;
	void* m_deallocstub = nullptr;
	void* m_serialize_stub = nullptr;
	std::unordered_map<void*, PyJitClassProxy*> m_instanceMap;

	JitClassType* m_pJitClassType = nullptr;
};