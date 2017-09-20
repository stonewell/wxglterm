#include <iostream>

#include <pybind11/embed.h>

#include "controller.h"
#include "plugin_manager_impl.h"

namespace py = pybind11;

std::shared_ptr<PluginManager> LoadAllPlugins(const char * plugin_path)
{
    std::shared_ptr<PluginManager> plugin_manager { new PluginManagerImpl() };

    py::dict locals;

    py::print(py::cast(plugin_path));
    py::print(py::cast(plugin_manager));

    locals["plugin_manager"] = py::cast(plugin_manager);
    locals["plugin_path"]    = py::cast(plugin_path);

    py::eval<py::eval_statements>(
        "def load_all_plugins(pm, p):\n"
        "  import os\n"
        "  import glob\n"
        "  for f in glob.glob(os.path.join(p, '*.*')):\n"
        "    print(f)\n"
        "f=load_all_plugins",
        py::globals(),
        locals);

    py::print(locals["f"]);
    locals["f"].cast<py::function>()(py::cast(plugin_manager), plugin_path);

    return plugin_manager;
}
