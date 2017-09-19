#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "controller.h"
#include "plugin_manager_impl.h"

namespace py = pybind11;

std::shared_ptr<PluginManager> LoadAllPlugins(const char * plugin_path)
{
    std::shared_ptr<PluginManager> plugin_manager { new PluginManagerImpl() };

    py::dict locals;
    locals["plugin_manager"] = py::cast(plugin_manager);
    locals["plugin_path"]        = py::cast(plugin_path);

    py::eval<py::eval_statements>(
        "import os\n"
        "import glob\n"
        "for f in glob.glob(os.path.join(plugin_path, '*.*')):\n"
        "  print(f)\n",
        py::globals(),
        locals);

    return plugin_manager;
}
