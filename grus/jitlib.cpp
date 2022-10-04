#include "jitlib.h"
#include "jitfuncinfo.h"
#include "jitclassinfo.h"
#include <iostream>
#include <fstream>
#include <regex>
#include "md5.h"
#include "GrusJitHost.h"
#include "utility.h"
#include "jitmgr.h"
#include "cppcompiler.h"
#include "cscompiler.h"
#include "javacompiler.h"

JitLib::JitLib()
{
}

JitLib::JitLib(std::string path, std::string libName)
{
	mPath = path;
	mSharedLibName = libName;
}

JitLib::~JitLib()
{
}

int JitLib::AddModule(std::string moduleName)
{
	int idx=0;
	auto it = mModuleMap.find(moduleName);
	if (it != mModuleMap.end())
	{
		idx = it->second;
	}
	else
	{
		mModuleNames.push_back(moduleName);
		idx = (int)mModuleNames.size()-1;
		mModuleMap.emplace(std::make_pair(moduleName, idx));
		mFuncMap.resize(mModuleMap.size());
	}
	return idx;
}

JitFuncPtr JitLib::QueryFunc(JitFuncInfo* pFuncInfo)
{
	int idx = pFuncInfo->ModuleIndex();
	if (idx < 0 || idx >= mFuncMap.size())
	{
		return nullptr;
	}
	auto& funcMap = mFuncMap[idx];
	auto it = funcMap.find(pFuncInfo->m_name);
	if (it != funcMap.end())
	{
		if (it->second.hash == pFuncInfo->m_hash)
		{
			return it->second.func;
		}
	}
	//make it if not existed or if changed with hash is not matched
	std::vector<std::string> out_files;
	GenAndBuild(out_files);
	it = funcMap.find(pFuncInfo->m_name);
	if (it != funcMap.end())
	{
		if (it->second.hash == pFuncInfo->m_hash)
		{
			return it->second.func;
		}
	}
	return nullptr;
}

void JitLib::AddBlock(int moduleIndex,std::string& name, JitFuncInfo* pFuncInfo,JitBlockType blockType)
{
	auto& funcMap = mFuncMap[moduleIndex];

	auto it = funcMap.find(name);
	if (it != funcMap.end())
	{
		it->second.pFuncInfo = pFuncInfo;
	}
	else
	{
		funcMap.emplace(std::make_pair(name, FuncInfo{"",nullptr,pFuncInfo,blockType,nullptr}));
	}
}

JitFuncInfo* JitLib::QueryFuncOrClassInfo(int nModuleIndex, std::string name)
{
	JitFuncInfo* pRetInfo = nullptr;
	auto& funcMap = mFuncMap[nModuleIndex];
	auto it = funcMap.find(name);
	if (it != funcMap.end())
	{
		pRetInfo = it->second.pFuncInfo;
	}
	return pRetInfo;
}

void JitLib::AddFuncPtr(int moduleIndex,const char* funcName, const char* hash, void* funcPtr)
{
	auto& funcMap = mFuncMap[moduleIndex];
	std::string name(funcName);
	auto it = funcMap.find(name);
	if (it != funcMap.end())
	{
		it->second.func = (JitFuncPtr)funcPtr;
		it->second.hash = hash;
	}
	else
	{
		funcMap.emplace(std::make_pair(name, FuncInfo{ hash,(JitFuncPtr)funcPtr,nullptr,JitBlockType::Func,nullptr }));
	}
}

void JitLib::AddClassPtr(int moduleIndex, const char* className, const char* hash,
	int propNum, int methodNum,
	std::vector<std::string>& classMemberNames,
	std::vector<unsigned long long>& classStubFuncs)
{
	auto& funcMap = mFuncMap[moduleIndex];
	std::string name(className);
	auto it = funcMap.find(name);
	JitClassInfo* pClassInfo = nullptr;
	if (it != funcMap.end())
	{
		it->second.hash = hash;
		pClassInfo = (JitClassInfo*)it->second.pFuncInfo;
	}
	else
	{
		//lib loaded before the class scaned from Python
		//so no class information created
		pClassInfo = new JitClassInfo();
		pClassInfo->SetLib(this);
		pClassInfo->SetModuleIndex(moduleIndex);
		funcMap.emplace(std::make_pair(name, FuncInfo{ hash,nullptr,pClassInfo,JitBlockType::Class,nullptr }));
	}
	pClassInfo->SetStubs(propNum, methodNum, classMemberNames, classStubFuncs);
	pClassInfo->SetStoredHash(hash);
}

std::string JitLib::QuotePath(std::string& strSrc)
{
	std::string strNew = strSrc;
	ReplaceAll(strNew, "\\", "\\\\");
	strNew = "\"" + strNew + "\"";
	return strNew;
}

void JitLib::SetHaveCppFunc(bool b, std::string& libFileName)
{
	if (b && mCppCompiler == nullptr)
	{
		mCppCompiler = new CppCompiler();
		mCppCompiler->SetLib(this);
		mCppCompiler->Init(libFileName);
	}
}

void JitLib::SetHaveCsFunc(bool b)
{
	if(b && mCsCompiler == nullptr)
	{
		mCsCompiler = new CsCompiler();
		mCsCompiler->SetLib(this);
		std::string defValue;
		mCsCompiler->Init(defValue);
	}
}

void JitLib::SetHaveJavaFunc(bool b)
{
	if (b && mJavaCompiler == nullptr)
	{
		mJavaCompiler = new JavaCompiler();
		mJavaCompiler->SetLib(this);
		std::string defVal;
		mJavaCompiler->Init(defVal);
	}
}

JitClassInfo* JitLib::FindClassFromNameSpace(std::string& moduleName, std::string& className)
{
	JitClassInfo* pClassInfo = nullptr;
	auto it = mModuleMap.find(moduleName);
	if (it != mModuleMap.end())
	{
		int idx = it->second;
		auto it2 = mFuncMap[idx].find(className);
		if (it2 != mFuncMap[idx].end())
		{
			pClassInfo =  (JitClassInfo*)it2->second.pFuncInfo;
		}
	}

	return pClassInfo;
}

void JitLib::UnloadLibs()
{
	if (mCppCompiler)
	{
		mCppCompiler->UnloadLib();
	}
	if (mCsCompiler)
	{
		mCsCompiler->UnloadLib();
	}
	if (mJavaCompiler)
	{
		mJavaCompiler->UnloadLib();
	}

}
bool JitLib::LoadLib(std::string libFileName)
{
	bool bOK = true;
	JitHost* pHost = &GrusJitHost::I();
	if (mCppCompiler)
	{
		bOK = mCppCompiler->LoadLib(libFileName, pHost);
	}
	else if (mCsCompiler)
	{
		bOK = mCsCompiler->LoadLib(libFileName, pHost);
	}
	else if (mJavaCompiler)
	{
		bOK = mJavaCompiler->LoadLib(libFileName, pHost);
	}
	return bOK;
}
bool JitLib::GenAndBuild(std::vector<std::string>& out_files,
	bool bGenerateCodeOnly,
	const char* outputFolder)
{
	std::string moduleList;
	for (auto& m : mModuleNames)
	{
		moduleList += m+" ";
	}
	printf("PyJit is Rebuilding %s\n", moduleList.c_str());

	std::string outputPath;
	if (outputFolder)
	{
		outputPath = outputFolder;
	}
	else
	{
		outputPath = mPath;
	}
	std::string strJitFolder = outputPath + Path_Sep + "_jit_";
	std::string strJitSrcFolder = strJitFolder + Path_Sep + "src";
	_mkdir(strJitFolder.c_str());
	_mkdir(strJitSrcFolder.c_str());

	std::vector<std::string> srcs;
	std::vector<std::string> exports;
	int num = (int)mModuleMap.size();
	for (int i = 0; i < num; i++)
	{//TODO: if one folder with cpp,cs and java
		//will cause problem
		//need to change here if cs and java are ready
		if (mCppCompiler)
		{
			mCppCompiler->BuildCode(i,strJitFolder, srcs, exports);
		}
		if (mCsCompiler)
		{
			mCsCompiler->BuildCode(i,strJitFolder, srcs, exports);
		}
		if (mJavaCompiler)
		{
			mJavaCompiler->BuildCode(i,strJitFolder, srcs, exports);
		}
	}
	out_files = srcs;
	if (bGenerateCodeOnly)
	{
		return true;
	}
	UnloadLibs();
	if (mCppCompiler)
	{
		mCppCompiler->CompileAndLink(strJitFolder, srcs, exports);
	}
	if (mCsCompiler)
	{
		mCsCompiler->CompileAndLink(strJitFolder, srcs, exports);
	}
	if (mJavaCompiler)
	{
		mJavaCompiler->CompileAndLink(strJitFolder, srcs, exports);
	}
	printf("PyJit finished Building\n");
	LoadLib();
	printf("PyJit Reloaded\n");
	return true;
}
