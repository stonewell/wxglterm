#pragma once

py::object LoadModuleFromFile(const char * file_path);
py::object LoadModuleFromString(const char * content, const char * module_name, const char * module_file);
