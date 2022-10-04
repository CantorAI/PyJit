#pragma once

#include <string>
#include <vector>
#include "grus_impl.h"

std::string ConvertFromObject(PyObject* pOb);
void _mkdir(const char* dir);
void ReplaceAll(std::string& data, std::string toSearch, std::string replaceStr);
std::wstring s2ws(const std::string& str);
std::string ws2s(const std::wstring& wstr);
bool exists(const std::string& name);
bool dir(std::string search_pat,
    std::vector<std::string>& subfolders,
    std::vector<std::string>& files);
bool IsAbsPath(std::string& path);
bool SplitPath(std::string& path, std::string& leftPath, std::string& rightPath);
void MakeOSPath(std::string& path);
std::vector<std::string> split(const std::string& str, char delim);
bool ParsePythonFunctionCode(std::string& wholecode,std::string& funcBody);
std::string& rtrim(std::string& s);
std::string& ltrim(std::string& s);
std::string& trim(std::string& s);
std::string GetCallerPyModuleFolder();
std::string GetRelativePath(const std::string& to, const std::string& from);