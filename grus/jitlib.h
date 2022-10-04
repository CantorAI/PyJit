#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include "grus_impl.h"
#include "compiler.h"

//TODO: change void* to predefined such as PYObj 
typedef void* (*JitFuncPtr)(void* vars);

class JitHost;
typedef int (*Jit_Init_Proc)(JitHost* pHost,void* context);
class JitFuncInfo;
class JitClassInfo;

enum class JitBlockType
{
	Func,
	Class
};
struct FuncInfo
{
	std::string hash;//if compiled,then set this
	JitFuncPtr func;
	JitFuncInfo* pFuncInfo = nullptr;
	JitBlockType blockType;
	void* libHandle = nullptr;//point to cpp lib,cs lib and java lib
};
enum class LangType
{
	undefine,
	cpp,
	cs,
	java
};

class JitLib
{
public:
	JitLib();
	JitLib(std::string path,std::string libName);
	~JitLib();
	int AddModule(std::string moduleName);

	JitFuncPtr QueryFunc(JitFuncInfo* pFuncInfo);
	void AddBlock(int moduleIndex,std::string& name, JitFuncInfo* pFuncInfo, JitBlockType blockType);
	void AddFuncPtr(int moduleIndex,const char* funcName, const char* hash,void* funcPtr);
	void AddClassPtr(int moduleIndex,const char* className,const char* hash,
		int propNum, int methodNum,
		std::vector<std::string>& classMemberNames,
		std::vector<unsigned long long>& classStubFuncs);
	void SetDebug(bool b)
	{
		if (mBuildWithDebug == false)
		{
			mBuildWithDebug = b;
		}
	}
	void AddIncludeFile(LangType langType,std::string& strFile)
	{
		switch (langType)
		{
		case LangType::cpp:
			mCppCompiler->AddIncludeFile(strFile);
			break;
		case LangType::cs:
			mCsCompiler->AddIncludeFile(strFile);
			break;
		case LangType::java:
			mJavaCompiler->AddIncludeFile(strFile);
			break;
		default:
			break;
		}
	}
	std::string LibFileName()
	{
		return mSharedLibName;
	}
	std::string ModuleName(int moduleIndex)
	{
		return mModuleNames[moduleIndex];
	}
	std::unordered_map<std::string, FuncInfo>& FuncMap(int moduleIndex)
	{
		return mFuncMap[moduleIndex];
	}
	bool LoadLib(std::string libFileName = "");
	bool GenAndBuild(std::vector<std::string>& out_files,
		bool bGenerateCodeOnly = false,const char* outputFolder=nullptr);

	void SetHaveCppFunc(bool b, std::string& libFileName);
	void SetHaveCsFunc(bool b);
	void SetHaveJavaFunc(bool b);

	std::string& Path() { return mPath; }
	std::string QuotePath(std::string& strSrc);
	bool IsBuildWithDebug() { return mBuildWithDebug; }
	JitFuncInfo* QueryFuncOrClassInfo(int nModuleIndex, std::string name);
	void AddIncludeDir(std::string& dir)
	{
		mIncludeDirs.push_back(dir);
	}
	std::vector<std::string>& IncludeDirs()
	{
		return mIncludeDirs;
	}
	JitClassInfo* FindClassFromNameSpace(std::string& moduleName, std::string& className);
private:
	bool mBuildWithDebug = false;
	void UnloadLibs();
	std::string mSharedLibName ="_m_";//just in case path is in wrong format
	//use python file name( no path) as key, and create an index
	std::vector<std::string> mModuleNames;
	std::unordered_map<std::string, int> mModuleMap;
	std::vector<std::unordered_map<std::string, FuncInfo>> mFuncMap;//keep same size with mModuleMap
	std::string mPath;//each path will create a _jit_ folder with one shared lib
	std::vector<std::string> mIncludeDirs;

	//cpp
	JitCompiler* mCppCompiler = nullptr;
	//cs
	JitCompiler* mCsCompiler = nullptr;
	//java
	JitCompiler* mJavaCompiler = nullptr;
};