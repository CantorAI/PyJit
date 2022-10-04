#include "jitfuncinfo.h"
#include "jitlib.h"
#include <iostream>
#include <fstream>
#include "md5.h"
#include "GrusJitHost.h"
#include "utility.h"
#include "jitmgr.h"
#include <regex>

static std::string calc_hash(std::string& strContent)
{
	return md5(strContent);
}


JitFuncInfo::JitFuncInfo()
{
}

JitFuncInfo::~JitFuncInfo()
{
}

void JitFuncInfo::AddCfg(std::string key, PyJit::Object Val)
{
	m_cfg.emplace(std::make_pair(key, Val));
}

LangType JitFuncInfo::SetLang(std::string& strType)
{
	LangType tp = LangType::undefine;

	if (strType.compare("cpp") == 0)
	{
		tp = LangType::cpp;
		std::string empty;
		mJitLib->SetHaveCppFunc(true, empty);
	}
	else if (strType.compare("cs") == 0)
	{
		tp = LangType::cs;
		mJitLib->SetHaveCsFunc(true);
	}
	else if (strType.compare("java") == 0)
	{
		tp = LangType::java;
		mJitLib->SetHaveJavaFunc(true);
	}
	m_lang = tp;
	return tp;
}

void JitFuncInfo::SetCode(std::string& strCode)
{
	m_code = strCode;
}

void JitFuncInfo::SetName(std::string& name)
{
	m_name = name;
	if (mJitLib)
	{
		mJitLib->AddBlock(m_moduleIndex,name, this,JitBlockType::Func);
	}
}

PyObject* JitFuncInfo::Call(PyObject* args, PyObject* kwargs)
{
	PyObject* pyRet = nullptr;
	JitFuncPtr func = mJitLib->QueryFunc(this);
	if (func)
	{
		pyRet = (PyObject*)func(args);
	}
	else
	{
		pyRet = Py_None;
		Py_IncRef(pyRet);
	}
	return pyRet;
}

bool JitFuncInfo::IsExternImpl()
{
	auto it = m_cfg.find(implKey);
	return (it != m_cfg.end());
}

std::vector<std::string> JitFuncInfo::GetExternImplFileName()
{
	std::vector<std::string> srcs;
	auto it = m_cfg.find(implKey);
	if (it != m_cfg.end())
	{
		PyJit::Object nameTuple = it->second;
		int cnt = (int)nameTuple.GetCount();
		for (int i = 0; i < cnt; i++)
		{
			std::string filename = (std::string)nameTuple[i];
			if (filename.empty())
			{
				continue;
			}
			MakeOSPath(filename);

			bool isWildPath = (filename.find_first_of("*?") >= 0);
			//check if filename is full path with prefix '/' or '\\'
			if (!IsAbsPath(filename))
			{
				filename = mJitLib->Path() + Path_Sep_S + filename;
			}
			if (isWildPath)
			{
				std::vector<std::string> subfolders;
				std::vector<std::string> files;
				dir(filename, subfolders, files);
				std::string leftPath, righFile;
				SplitPath(filename, leftPath, righFile);
				for (auto s : files)
				{
					srcs.push_back(leftPath+ Path_Sep_S+s);
				}
			}
			else
			{
				srcs.push_back(filename);
			}
		}
	}
	return srcs;
}
std::vector<std::string> JitFuncInfo::GetIncludesFileName()
{
	std::vector<std::string> srcs;
	for (auto& filename :mIncludeFiles)
	{
		if (filename.empty())
		{
			continue;
		}
		//check if filename is full path with prefix '/' or '\\'
		if (filename[0] != Path_Sep)
		{
#if (!WIN32)
			ReplaceAll(filename, "\\", "/");
#endif
			filename = mJitLib->Path() + Path_Sep_S + filename;
		}
		srcs.push_back(filename);
	}
	return srcs;
}
/*
this function uses __annotations__ and __defaults__ 
but it can't parse line below correctly
	def cpp_func(x,y:float,z,d):
*/
void JitFuncInfo::ParseFuncInfo_use_annotations(std::string& name,
	PyJit::Object& funcObject,
	ClassFuncInfo& funcInfo,
	std::string& strfuncDesc)
{
	funcInfo.name = name;

	PyJit::Dict func_annotations = (PyJit::Object)funcObject["__annotations__"];
	PyJit::Object func_defaults = (PyJit::Object)funcObject["__defaults__"];
	funcInfo.returnType = (std::string)func_annotations["return"];

	PyJit::Object func_keys = func_annotations.Keys();
	int num = (int)func_keys.GetCount();
	int def_num = (int)func_defaults.GetCount();
	for (int j = 0; j < num; j++)
	{
		std::string p_name = (std::string)func_keys[j];
		if (p_name == "return")
		{
			continue;
		}
		std::string p_clsType = (std::string)func_annotations[p_name.c_str()];
		std::string p_defValue;
		if (j < def_num)
		{
			p_defValue = (std::string)func_defaults[j];
		}
		funcInfo.parameters.push_back({ p_name,p_clsType,p_defValue });
	}
	strfuncDesc = (std::string)funcObject["__doc__"];
}
bool JitFuncInfo::ParseFuncInfo(std::string& code,ClassFuncInfo& funcInfo,std::string& strfuncBody)
{
	bool bRet = false;
	//(?:) is non-capture group
	static std::regex rgx("\\t*def[\\t\\s]+([^\\(\\)]+)\\(([^\\\\(\\\\)]*)\\)[\\s\\t]*(?:->[\\s\\t]*([^:]+))?:");
	//to parse def func_name([var:type]*)->var:
	std::smatch matches;
	if (std::regex_search(code, matches, rgx))
	{
		if (matches.size() >= 3)
		{
			std::string funcName = matches[1].str();
			funcInfo.name = funcName;
			std::string strVars = matches[2].str();
			std::vector<std::string> vars = split(strVars, ',');

			for(auto var:vars)
			{
				//format: {varName:varType=defaultValue}*
				std::string varName, varType, defVal;
				auto pos = var.find(":");
				bool bFindName = false;
				if (pos != std::string::npos)
				{
					varName = var.substr(0, pos);
					bFindName = true;
					var = var.substr(pos+1);
				}
				pos = var.find("=");
				if (pos != std::string::npos)
				{
					if (bFindName)
					{
						varType = var.substr(0, pos);
					}
					else
					{
						varName = var.substr(0, pos);
						bFindName = true;
					}
					defVal = var.substr(pos + 1);
				}
				else
				{
					if (bFindName)
					{
						varType = var;
					}
				}
				if (!bFindName)
				{
					varName = var;
				}
				if (varName != "self")
				{
					trim(varName);
					trim(varType);
					int nSize = (int)varType.size();
					if (nSize>=2)
					{
						if (varType[0] == '\"' && varType[nSize-1] =='\"')
						{
							varType = varType.substr(1,nSize-2);
						}
					}

					trim(defVal);
					funcInfo.parameters.push_back
					(
						{
							varName,varType,defVal,nullptr,nullptr
						}
					);
				}
			}
			if (matches.size() == 4)
			{
				std::string retType = matches[3].str();
				int nSize = (int)retType.size();
				if (nSize >= 2)
				{
					if (retType[0] == '\"' && retType[nSize - 1] == '\"')
					{
						retType = retType.substr(1, nSize - 2);
					}
				}
				funcInfo.returnType = retType;
			}
			std::string body = matches.suffix();
			std::regex commentTag("\"\"\"");
			strfuncBody = std::regex_replace(body, commentTag, "");
			bRet = true;
		}
	}
	return bRet;
}
/*
	bind format
	__bind__:class's variable information;__default__:default_value
	for example:
	body:int ="__bind__:m_body;__default__:100"
	make:str ='__bind__:m_make;__default__:"Audi"'
*/
bool JitFuncInfo::ParseBindInfo(std::string strInfo, VarInfo& varInfo)
{
	bool haveBindInfo = false;
	auto list = split(strInfo, ';');
	for (auto i : list)
	{
		i = trim(i);
		auto parts = split(i, ':');
		if (parts.size() == 2)
		{
			auto tag = trim(parts[0]);
			auto val = trim(parts[1]);
			if (tag == "__bind__")
			{
				varInfo.bindto = val;
				haveBindInfo = true;
			}
			else if (tag == "__default__")
			{
				varInfo.defaultValue = val;
			}
		}
	}
	return haveBindInfo;
}
std::string JitFuncInfo::MakeFileChangeTimeSpec(std::string filename)
{
	#include <sys/types.h>
	#include <sys/stat.h>
	#ifndef WIN32
	#include <unistd.h>
	#endif

	#ifdef WIN32
	#define stat _stat
	#endif

	std::string retStr;
	struct stat result;
	if (stat(filename.c_str(), &result) == 0)
	{
		auto mod_time = result.st_mtime;
		const int online_len = 1000;
		char funcLine[online_len];
#ifdef WIN32
		SPRINTF(funcLine, online_len, "%s:%llx\n", filename.c_str(), mod_time);
#else
		SPRINTF(funcLine, online_len, "%s:%lx\n", filename.c_str(), mod_time);
#endif
		retStr = funcLine;
	}
	return retStr;
}
void JitFuncInfo::BuildRelativeFileSpec(std::string& spec)
{
	std::vector<std::string> includeFiles = GetIncludesFileName();
	for (auto& i : includeFiles)
	{
		spec += MakeFileChangeTimeSpec(i);
	}
	std::vector<std::string> srcFiles = GetExternImplFileName();
	for (auto& i : srcFiles)
	{
		spec += MakeFileChangeTimeSpec(i);
	}
}
