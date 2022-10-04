#include "jitclassinfo.h"
#include <algorithm>

JitClassInfo::JitClassInfo()
{
}

JitClassInfo::~JitClassInfo()
{
}

void JitClassInfo::Reset()
{
	m_props.clear();
	m_classfuncs.clear();
	m_IsClassStubLoaded = false;
}

void JitClassInfo::BuildClassInfo(PyObject* obFunc, std::string& className,
	std::string& fingerprints)
{
	PyJit::Object inspect = PyJit::Object::Import("inspect");
	PyJit::Object classObj(obFunc,true);
	PyJit::Dict annotations = (PyJit::Object)classObj["__annotations__"];
	PyJit::Object annotations_keys = annotations.Keys();

	//fill into temp array first, because we need to check if the lib loaded
	//and see if already have these arrays
	std::vector<VarInfo> props;
	std::vector<ClassFuncInfo> classfuncs;

	int cnt = (int)annotations_keys.GetCount();
	for (int i = 0; i < cnt; i++)
	{
		std::string name = (std::string)annotations_keys[i];
		std::string clsType = (std::string)annotations[name.c_str()];
		props.push_back({ name,clsType,"" });
		fingerprints += name +"->" + clsType+"\n";
		//printf("name:%s,clsType:%s\r\n", name.c_str(), clsType.c_str());
	}

	PyJit::Object getsourceFunc = (PyJit::Object)inspect["getsource"];
	PyJit::Object members = inspect["getmembers"](classObj);
	cnt = (int)members.GetCount();
	for (int i = 0; i < cnt; i++)
	{
		PyJit::Object m = (PyJit::Object)members[i];
		std::string name = (std::string)m[0];
		//printf("Key=%s\r\n", name.c_str());
		if (name == m_init_func_name)
		{
			PyJit::Object funcObj = (PyJit::Object)m[1];
			std::string wholecode;
			try
			{
				wholecode = (std::string)getsourceFunc(funcObj);
			}
			catch (...)
			{
				PyErr_Print();
			}
			fingerprints += wholecode + "\n";
			std::string funcBody;
			ParseFuncInfo(wholecode,m_initfuncInfo, funcBody);
			m_have_init_func = true;
			continue;
		}
		bool IsSysVar = (name.size() > 4 && name[0] == '_' && name[1] == '_'
			&& name[name.size() - 1] == '_' && name[name.size() - 2] == '_');
		if (IsSysVar)
		{
			continue;
		}
		PyJit::Object val = (PyJit::Object)m[1];
		int isFunc = (int)inspect["isfunction"](val);
		if (isFunc)
		{
			ClassFuncInfo funcInfo;
			std::string strfuncDesc;
			std::string wholecode = (std::string)getsourceFunc(val);
			fingerprints += wholecode + "\n";
			std::string funcBody;
			ParseFuncInfo(wholecode, funcInfo, funcBody);
			classfuncs.push_back(funcInfo);
		}
		else
		{
			for (auto& prop : props)
			{
				if (prop.name == name)
				{
					std::string defVal = (std::string)val;
					fingerprints += defVal + "\n";
					if (!ParseBindInfo(defVal,prop))
					{
						prop.defaultValue = defVal;
					}
					break;
				}
			}
		}
	}
	//check if class members have funcation pointers 
	if (m_props.size() == props.size())//code geneated by PYJIT, should be matched with orders
	{
		for (int i = 0; i < props.size(); i++)
		{
			assert(m_props[i].name == props[i].name);
			m_props[i].type = props[i].type;
			m_props[i].defaultValue = props[i].defaultValue;
		}
	}
	else if(m_props.size() ==0)
	{
		m_props = props;
	}
	if (m_classfuncs.size() == classfuncs.size())//code geneated by PYJIT, should be matched with orders
	{
		for (int i = 0; i < classfuncs.size(); i++)
		{
			assert(m_classfuncs[i].name == classfuncs[i].name);
			m_classfuncs[i].returnType = classfuncs[i].returnType;
			m_classfuncs[i].parameters = classfuncs[i].parameters;
		}
	}
	else if (m_classfuncs.size() == 0)
	{
		m_classfuncs = classfuncs;
	}
}
void JitClassInfo::CopyNecessaryPropsFromWrapperFuncInfo(JitFuncInfo* pWrapperFuncInfo)
{
	const char* lib_dir_tag = "lib_dir";

	m_lang = pWrapperFuncInfo->Lang();
	auto& cfg = pWrapperFuncInfo->GetCfg();
	for (auto& it : cfg)
	{
		std::string strKeyLower = it.first;
		std::transform(strKeyLower.begin(),
			strKeyLower.end(), strKeyLower.begin(), ::tolower);
		if (strKeyLower == "serialization")
		{
			m_support_serialization = (bool)(PyJit::Object)it.second;
		}
		else
		{
			AddCfg(it.first, it.second);
		}
	}
	auto& incFiles = pWrapperFuncInfo->IncludeFiles();
	for (auto s : incFiles)
	{
		mIncludeFiles.push_back(s);
	}
	m_className = pWrapperFuncInfo->ClassName();
}
/*
stubs = class_new +
		class_dealloc +
		serialize_stub +
		{Prop.get+Prop.set}*
		{Method Stub}*
*/
void JitClassInfo::SetStubs(int propNum, int methodNum, 
	std::vector<std::string>& classMemberNames,
	std::vector<unsigned long long>& stubs)
{
	const int offset = 3;
	m_newstub = (void*)stubs[0];
	m_deallocstub = (void*)stubs[1];
	m_serialize_stub = (void*)stubs[2];
	//two cases, first: if m_props.size() ==0 && m_classfuncs.size() ==0
	//means not getting class info by parsing python class
	//so fill m_props and m_classfuncs with this call, incuding name and function pointers
	//second: class info loaded, and not set funcation pointers
	if (m_props.size() == 0 && m_classfuncs.size() == 0)
	{
		if (propNum > 0)
		{
			m_props.resize(propNum);
			for (int i = 0; i < propNum; i++)
			{
				VarInfo& varInfo = m_props[i];
				varInfo.name = classMemberNames[offset + i];
				varInfo.getter = (void*)stubs[offset + i * 2];
				varInfo.setter = (void*)stubs[offset + i * 2 + 1];
			}
		}
		if (methodNum > 0)
		{
			m_classfuncs.resize(methodNum);
			for (int i = 0; i < methodNum; i++)
			{
				auto& funcInfo = m_classfuncs[i];
				funcInfo.name = classMemberNames[offset + propNum + i];
				funcInfo.stubfunc = (void*)stubs[offset + propNum * 2 + i];
			}
		}
	}
	else if(propNum == m_props.size() && methodNum == m_classfuncs.size())
	{
		for (int i = 0; i < propNum; i++)
		{
			VarInfo& varInfo = m_props[i];
			varInfo.getter = (void*)stubs[offset + i * 2];
			varInfo.setter = (void*)stubs[offset + i * 2 + 1];
		}
		for (int i = 0; i < methodNum; i++)
		{
			auto& funcInfo = m_classfuncs[i];
			funcInfo.stubfunc = (void*)stubs[offset + propNum * 2 + i];
		}
	}//else, some errors happened
	m_IsClassStubLoaded = true;
}

void JitClassInfo::SetStoredHash(const char* hash)
{
	m_HashStored = hash;
}

void JitClassInfo::SetHash(std::string& h)
{
	JitFuncInfo::SetHash(h);
	if (!m_HashStored.empty() && m_HashStored != h)
	{//class code changed, but lib loaded,need to reset
		Reset();
	}
}

void JitClassInfo::SetName(std::string& name)
{
	m_name = name;
	if (mJitLib)
	{
		mJitLib->AddBlock(m_moduleIndex, name, this, JitBlockType::Class);
	}
}

std::string JitClassInfo::GetFullNameSpace()
{
	return mJitLib->LibFileName() + "." + mJitLib->ModuleName(m_moduleIndex);
}
