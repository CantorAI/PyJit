#include "utility.h"
#include <cctype>
#include <codecvt>
#include <locale>
#include <sstream>
#include <vector>
#include <regex>
#include "Jit_Object.h"
#include "gxydef.h"
#include "port.h"

std::string ConvertFromObject(PyObject* pOb)
{
	std::string strVal;
	if (pOb == nullptr)
	{
		return strVal;
	}
	if (PyUnicode_Check(pOb))
	{
#ifndef PythonType2
		strVal = PyUnicode_AsUTF8(pOb);
#else
		strVal = PyString_AsString(pOb);
#endif
	}
	else
	{
		PyObject* strOb = PyObject_Repr(pOb);
#ifndef PythonType2
		strVal = PyUnicode_AsUTF8(strOb);
#else
		strVal = PyString_AsString(strOb);
#endif
		Py_DECREF(strOb);

	}
	return strVal;
}

#if (WIN32)
#include <Windows.h>
void _mkdir(const char* dir)
{
    CreateDirectory(dir, NULL);
}
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

void _mkdir(const char* dir)
{
    int state = access(dir, R_OK | W_OK);
    if (state != 0)
    {
        mkdir(dir, S_IRWXU);
    }
}
#endif

void ReplaceAll(std::string& data, std::string toSearch, std::string replaceStr)
{
    // Get the first occurrence
    size_t pos = data.find(toSearch);
    // Repeat till end is reached
    while (pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos = data.find(toSearch, pos + replaceStr.size());
    }
}
std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}
std::string ws2s(const std::wstring& wstr)
{
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;
    std::string ret = converter.to_bytes(wstr);
    return ret;
}
bool exists(const std::string& name) 
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}
bool IsAbsPath(std::string& path)
{
	if (path.size() == 0)
	{
		return false;
	}
	bool ret = false;
#if (WIN32)
	if ((path.find_first_of(":") !=std::string::npos) || (path[0] == '/') || (path[0] == '\\'))
	{
		ret = true;
	}
#else
	if (path[0] == '/')
	{
		ret = true;
	}
#endif
	return ret;
}
void MakeOSPath(std::string& path)
{
#if (WIN32)
	ReplaceAll(path, "/", "\\");
#else
	ReplaceAll(path, "\\", "/");
#endif
}
bool SplitPath(std::string& path, std::string& leftPath, std::string& rightPath)
{
	size_t pos = 0;
#if (WIN32)
	pos = path.find_last_of("\\/");
#else
	pos = path.find_last_of("/");
#endif
	if (pos != std::string::npos)
	{
		leftPath = path.substr(0, pos);
		rightPath = path.substr(pos + 1);
	}
	else
	{
		leftPath = "";
		rightPath = path;
	}
	return true;
}
bool dir(std::string search_pat,
	std::vector<std::string>& subfolders,
    std::vector<std::string>& files)
{
	bool ret = false;
#if (WIN32)
	BOOL result = TRUE;
	WIN32_FIND_DATA ff;

	HANDLE findhandle = FindFirstFile(search_pat.c_str(), &ff);
	if (findhandle != INVALID_HANDLE_VALUE)
	{
		ret = true;
		BOOL res = TRUE;
		while (res)
		{
			// We only want directories
			if (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				std::string fileName(ff.cFileName);
				if (fileName != "." && fileName != "..")
				{
					subfolders.push_back(fileName);
				}
			}
			else
			{
				std::string fileName(ff.cFileName);
				files.push_back(fileName);
			}
			res = FindNextFile(findhandle, &ff);
		}
		FindClose(findhandle);
	}
#else
#include <dirent.h>
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir(search_pat.c_str())) != NULL)
	{
		ret = true;
		while ((ent = readdir(dir)) != NULL) 
		{
			if (ent->d_type == DT_DIR)
			{
				std::string fileName(ent->d_name);
				if (fileName != "." && fileName != "..")
				{
					subfolders.push_back(fileName);
				}
			}
			else if (ent->d_type == DT_REG)//A regular file
			{
				std::string fileName(ent->d_name);
				files.push_back(fileName);
			}
		}
		closedir(dir);
	}
#endif
	return ret;
}
std::vector<std::string> split(const std::string& str, char delim)
{
	std::vector<std::string> list;
	std::string temp;
	std::stringstream ss(str);
	while (std::getline(ss, temp, delim))
	{
		list.push_back(temp);
	}
	return list;
}

bool ParsePythonFunctionCode(std::string& wholecode, std::string& funcBody)
{
	bool bRet = false;
	//(?:) is non-capture group
	static std::regex rgx("\\t*def[\\t\\s]+([^\\(\\)]+)\\(([^\\\\(\\\\)]*)\\)[\\s\\t]*(?:->[\\s\\t]*([^:]+))?:");
	static std::regex rgx_var("([^:,]+)(?::([^,]+))?,?");
	//to parse def func_name([var:type]*)->var:
	std::smatch matches;
	if (std::regex_search(wholecode, matches, rgx))
	{
		if (matches.size() >= 3)
		{
			std::string body = matches.suffix();
			std::regex commentTag("\"\"\"");
			funcBody = std::regex_replace(body, commentTag, "");
			bRet = true;
		}
	}
	return bRet;
}
#define TRIMOFF_CHARS " \t\n\r\f\v"

// trim from end of string (right)
std::string& rtrim(std::string& s)
{
	s.erase(s.find_last_not_of(TRIMOFF_CHARS) + 1);
	return s;
}

// trim from beginning of string (left)
std::string& ltrim(std::string& s)
{
	s.erase(0, s.find_first_not_of(TRIMOFF_CHARS));
	return s;
}

// trim from both ends of string (right then left)
std::string& trim(std::string& s)
{
	return ltrim(rtrim(s));
}
std::string GetCallerPyModuleFolder()
{
	std::string modulePath;
	PyJit::Object locals(PyEval_GetLocals(), true);
	PyJit::Object thisFile = (PyJit::Object)locals["__file__"];
	modulePath = (std::string)PyJit::Object()["os.path.abspath"](thisFile);
	auto pos = modulePath.rfind(Path_Sep);
	if (pos != std::string::npos)
	{
		modulePath = modulePath.substr(0, pos + 1);
	}
	return modulePath;
}

static inline bool StringsEqual_i(const std::string& lhs, const std::string& rhs)
{
	return STRCMPNOCASE(lhs.c_str(), rhs.c_str(),
		lhs.size()>rhs.size()?lhs.size(): rhs.size()) == 0;
}

static void SplitPath(const std::string& in_path, std::vector<std::string>& split_path)
{
	size_t start = 0;
	size_t dirsep;
	do
	{
		dirsep = in_path.find_first_of("\\/", start);
		std::string p;
		if (dirsep == std::string::npos)
			p = std::string(&in_path[start]);
		else
			p = std::string(&in_path[start], &in_path[dirsep]);
		if (p.size() > 0)
		{
			split_path.push_back(p);
		}
		start = dirsep + 1;
	} while (dirsep != std::string::npos);
}

/**
 * Get the relative path from a base location to a target location.
 *
 * \param to The target location.
 * \param from The base location. Must be a directory.
 * \returns The resulting relative path.
 */
std::string GetRelativePath(const std::string& to, const std::string& from)
{
	std::vector<std::string> to_dirs;
	std::vector<std::string> from_dirs;

	SplitPath(to, to_dirs);
	SplitPath(from, from_dirs);

	std::string output;
	output.reserve(to.size());

	std::vector<std::string>::const_iterator to_it = to_dirs.begin(),
		to_end = to_dirs.end(),
		from_it = from_dirs.begin(),
		from_end = from_dirs.end();

	while ((to_it != to_end) && (from_it != from_end) && StringsEqual_i(*to_it, *from_it))
	{
		++to_it;
		++from_it;
	}

	while (from_it != from_end)
	{
		output += "..\\";
		++from_it;
	}

	while (to_it != to_end)
	{
		output += *to_it;
		++to_it;

		if (to_it != to_end)
			output += "\\";
	}

	return output;
}
