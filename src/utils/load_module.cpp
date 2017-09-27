#include <pybind11/embed.h>
#include <iostream>
#include <string>

namespace py = pybind11;

#include "load_module.h"

py::object LoadPyModuleFromFile(const char * file_path)
{
    py::dict locals;
    locals["path"]  = py::cast(file_path);

    py::eval<py::eval_statements>(            // tell eval we're passing multiple statements
        "import imp\n"
        "import os\n"
        "module_name = os.path.basename(path)[:-3]\n"
        "new_module = imp.load_module(module_name, open(path), path, ('py', 'U', imp.PY_SOURCE))\n",
        py::globals(),
        locals);

    auto py_module = locals["new_module"];

    return py_module;
}

py::object LoadPyModuleFromString(const char * content, const char * module_name, const char * module_file)
{
    py::dict locals;

    locals["module_content"] = py::cast(content);
    locals["module_name"] = py::cast(module_name);
    locals["module_file"] = py::cast(module_file);

    py::object result = py::eval<py::eval_statements>(
        "import imp\n"
        "import os\n"
        "import io\n"
        "new_module = imp.load_module(module_name, io.StringIO(module_content), module_file, ('py', 'U', imp.PY_SOURCE))\n",
        py::globals(),
        locals);

    return locals["new_module"];
}
