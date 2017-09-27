#pragma once

#include "handles.h"
#include <pybind11/embed.h>

pybind11::object LoadPyModuleFromFile(const char * file_path);
pybind11::object LoadPyModuleFromString(const char * content, const char * module_name, const char * module_file);

Handle LoadDyModuleFromFile(const char * file_path);
void * GetDyProcAddress(Handle module, const char * func_name);
