#include <iostream>

#include <pybind11/embed.h>

#include "controller.h"
#include "plugin_manager_impl.h"

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(plugin_manager, m) {
    m.def("load", [](int i, int j) {
        return i + j;
    });
}

std::shared_ptr<PluginManager> LoadAllPlugins(const char * plugin_path)
{
    std::shared_ptr<PluginManager> plugin_manager { new PluginManagerImpl() };

    py::dict locals;
    //locals["plugin_manager"] = pm;
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
    locals["f"].cast<py::function>()(new PluginManagerImpl(), plugin_path);

    return plugin_manager;
}
