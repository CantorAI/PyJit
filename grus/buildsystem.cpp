#include "buildsystem.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "utility.h"

static std::vector<std::string> _configure_compile_keys_
{
	"output_dir",
	"macros",
	"include_dirs",
	"debug",
	"extra_preargs",
	"extra_postargs",
	"depends"
};
struct LinkKeyInfo
{
	std::string key;
	std::string KeyPassToLink;
};
static std::vector<LinkKeyInfo> _configure_link_keys_
{
	{"output_dir"},
	{"libraries"},
	{"library_dirs"},
	{"runtime_library_dirs"},
	{"debug"},
	{"link_extra_preargs","extra_preargs"},
	{"link_extra_postargs","extra_postargs"},
	{"target_lang"}
};
PyObject* BuildSystem::Build(PyObject* args, PyObject* kwargs)
{
	std::string curModuleFolder = GetCallerPyModuleFolder();
	PyJit::Object kwlist(kwargs);
	PyJit::Object sources = kwlist["srcs"];
	if (!sources.IsList())
	{//have to be list, if not, init to empty
		sources = PyJit::Object(std::vector<std::string>());
	}
	if (args)
	{
		PyJit::Object o_args(args);
		int cnt = (int)o_args.GetCount();
		for (int i = 0; i < cnt; i++)
		{
			PyJit::Object src_item = (PyJit::Object)o_args[i];
			if (src_item.IsList())
			{
				int list_cnt = (int)src_item.GetCount();
				for (int j = 0; j < list_cnt; j++)
				{
					PyList_Append((PyObject*)sources.ref(),
						(PyObject*)(GalaxyJitPtr)src_item[j]);
				}
			}
			else//todo: check
			{//string
				PyList_Append((PyObject*)sources.ref(),
					(PyObject*)(GalaxyJitPtr)src_item);
			}
		}
	}

	sources = ExpandWildCast(curModuleFolder,sources);

	std::map <std::string, PyJit::Object> configure_compile;
	std::map <std::string, PyJit::Object> configure_link;
	for (auto& key : _configure_compile_keys_)
	{
		PyJit::Object val = (PyJit::Object)kwlist[key.c_str()];
#if (WIN321)
		if (key == "macros")
		{
			std::vector<PyJit::Tuple> macros_win
			{
				PyJit::Tuple(std::vector<std::string>{"WIN32","1"})
			};
			if (val.IsNull())
			{
				val = macros_win;
			}
			else
			{
				PyJit::Object winList(macros_win);
				PyList_Append((PyObject*)val.ref(),
					(PyObject*)(GalaxyJitPtr)winList);
			}
		}
#endif
		if (!val.IsNull())
		{
			configure_compile.emplace(std::make_pair(key, val));
		}
	}
	for (auto& keyInfo : _configure_link_keys_)
	{
		PyJit::Object val = (PyJit::Object)kwlist[keyInfo.key.c_str()];
		if (!val.IsNull())
		{
			std::string passKey;
			if (keyInfo.KeyPassToLink.length() == 0)
			{
				passKey = keyInfo.key;
			}
			else
			{
				passKey = keyInfo.KeyPassToLink;
			}
			configure_link.emplace(std::make_pair(passKey, val));
		}
	}

	auto M_ccompiler = PyJit::Object::Import("distutils.ccompiler");
	auto compiler = M_ccompiler["new_compiler"]();
	auto M_sysconfig = PyJit::Object::Import("distutils.sysconfig");
	M_sysconfig["customize_compiler"](compiler);
	//GalaxyJitPtr pp = PyJit::Object(mJitLib->LibFileName()).ref();
	std::string target_link_func = "link_executable";
	auto objTarget_type = (PyJit::Object)kwlist["target_type"];
	auto target_name = (PyJit::Object)kwlist["target_name"];

	auto os = PyJit::Object::Import("os");
	std::string cwd = (std::string)os["getcwd"]();
	//change
	os["chdir"](curModuleFolder);

	if (!objTarget_type.IsNull())
	{
		std::string targetType = (std::string)objTarget_type;
		if (targetType.find("static")==0)
		{
			target_link_func = "create_static_lib";
		}
		else if(targetType.find("shared") == 0)
		{
			target_link_func = "link_shared_object";
			auto ptrs = target_name.ref();
			std::string taget_FileName = (std::string)((PyJit::Object)
				compiler["library_filename"]).Call(1, &ptrs,
					PyJit::Object(std::map <std::string, PyJit::Object>
			{
				{"lib_type", "shared"},
			}));
			target_name = PyJit::Object(taget_FileName);
		}
		else if(targetType.find("exe") == 0)
		{
			target_link_func = "link_executable";
		}
	}

	auto pp = sources.ref();
	auto objects = ((PyJit::Object)compiler["compile"]).Call(
		1, &pp, PyJit::Object(configure_compile));
	PyJit::Object objs = kwlist["objs"];
	if (objs.IsList())
	{
		if (!objects.IsList())
		{
			objects = PyJit::Object(std::vector<std::string>());
		}
		PyObject* pObList = (PyObject*)objs.ref();
		auto size = PyList_GET_SIZE(pObList);
		PyObject* allList = (PyObject*)objects.ref();
		for (int i = 0; i < size; i++)
		{
			PyList_Append(allList, PyList_GetItem(pObList,i));
		}
	}
	GalaxyJitPtr pp2[2] = 
	{	
		objects.ref(),
		target_name.ref()
	};
	//create_static_lib
	PyJit::Object(compiler[target_link_func.c_str()]).Call(
		2, pp2, PyJit::Object(configure_link));

	//restore
	os["chdir"](cwd);

	Py_RETURN_TRUE;
}
//Each Item can have format below
//filename>[excludes]

PyJit::Object BuildSystem::ExpandWildCast(std::string curFolder,PyJit::Object sources)
{
	std::vector<std::string> list;
	int cnt = (int)sources.GetCount();
	for (int i = 0; i < cnt; i++)
	{
		std::string item = (std::string)sources[i];
		list.push_back(item);
	}
	list = ParseFolder(curFolder,list);
	auto path = PyJit::Object::Import("os.path");
	for (auto& l : list)
	{
		if (!(bool)path["isabs"](l))
		{
			l = (std::string)path["abspath"](path["join"](curFolder,l));
		}
	}
	return PyJit::Object(list);
}

std::vector<std::string> BuildSystem::ParseFolder(
	std::string curFolder,
	std::vector<std::string>& srcs)
{
	std::vector<std::string> retList;
	auto glob2 = PyJit::Object::Import("glob2");
	for (auto& src : srcs)
	{
		//check format:filename>[excludes]
		std::string folder = src;
		std::vector<std::string> exclude_list;
		auto pos = src.find('>');
		if (pos != std::string::npos)
		{
			folder = folder.substr(0, pos);
			auto temp_src = src.substr(pos + 1);
			std::string excludes = trim(temp_src);
			if (excludes.size() >= 2 && excludes[0] == '['
				&& excludes[excludes.size() - 1] == ']')
			{
				excludes = excludes.substr(1, excludes.size() - 2);
				exclude_list = split(excludes, ',');
			}
			else
			{
				exclude_list.push_back(excludes);
			}
			exclude_list = ParseFolder(curFolder,exclude_list);
		}
		auto find_from_list = [](std::string& src,
			std::vector<std::string>& list)
		{
			bool bFind = false;
			for (auto it = list.begin();
				it != list.end(); it++)
			{
				if (src == *it)
				{
					bFind = true;
					break;
				}
			}
			return bFind;
		};
		if (folder.size() == 0)
		{//just have excludes,means excludes from previous list
			for (auto it0 = retList.begin(); 
				it0 != retList.end(); it0++)
			{
				if (find_from_list(*it0, exclude_list))
				{
					retList.erase(it0--);
				}
			}
			continue;
		}
		auto os = PyJit::Object::Import("os");
		std::string cwd = (std::string)os["getcwd"]();
		//change
		os["chdir"](curFolder);
		PyJit::Object expanded_items = glob2["glob"](folder);
		//restore
		os["chdir"](cwd);
		int items_cnt = (int)expanded_items.GetCount();
		for (int j = 0; j < items_cnt; j++)
		{
			std::string src_item = (std::string)expanded_items[j];
			if (!find_from_list(src_item, exclude_list))
			{
				retList.push_back(src_item);
			}
		}
	}
	return retList;
}
