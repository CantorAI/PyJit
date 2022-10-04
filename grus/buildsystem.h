#pragma once

#include "singleton.h"
#include "grus_impl.h"
#include "Jit_Object.h"
#include <vector>
#include <string>

class BuildSystem :
    public Singleton<BuildSystem>
{
public:
    PyObject* Build(PyObject* args, PyObject* kwargs);
    PyJit::Object ExpandWildCast(std::string curFolder,PyJit::Object sources);
    std::vector<std::string> ParseFolder(
        std::string curFolder,
        std::vector<std::string>& srcs);
};