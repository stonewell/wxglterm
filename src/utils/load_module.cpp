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
        "import importlib.util\n"
        "import os\n"
        "import sys\n"
        "module_name = os.path.basename(path)[:-3]\n"
        "try:\n"
        "  new_module = sys.modules[module_name]\n"
        "except KeyError:\n"
        "  spec = importlib.util.spec_from_file_location(module_name, path)\n"
        "  new_module = importlib.util.module_from_spec(spec)\n"
        "  spec.loader.exec_module(new_module)\n",
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
        "import importlib.util\n"
        "import os\n"
        "import io\n"
        "import sys\n"
        "import tempfile\n"
        "try:\n"
        "  new_module = sys.modules[module_name]\n"
        "except KeyError:\n"
        "  t_fd, t_path = tempfile.mkstemp(suffix='.py', text=True)\n"
        "  t_f = os.fdopen(t_fd, 'w')\n"
        "  t_f.write(module_content)\n"
        "  t_f.close()\n"
        "  spec = importlib.util.spec_from_file_location(module_name, t_path)\n"
        "  new_module = importlib.util.module_from_spec(spec)\n"
        "  spec.loader.exec_module(new_module)\n"
        "  os.remove(t_path)\n",
        py::globals(),
        locals);

    return locals["new_module"];
}
