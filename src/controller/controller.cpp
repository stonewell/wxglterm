#include <pybind11/embed.h>

#include "controller.h"
#include "plugin_manager_impl.h"

namespace py = pybind11;

std::shared_ptr<PluginManager> LoadAllPlugins(const char * plugin_path)
{
    std::shared_ptr<PluginManager> plugin_manager { new PluginManagerImpl() };

    py::dict locals;

    locals["plugin_manager"] = plugin_manager;
    locals["plugin_path"]    = py::cast(plugin_path);

    py::eval<py::eval_statements>(
        "def load_all_plugins(pm, p):\n"
        "  import os\n"
        "  import glob\n"
        "  for f in glob.glob(os.path.join(p, '*.*')):\n"
        "    pm.register_plugin(f)\n"
        "load_all_plugins(plugin_manager, plugin_path)",
        py::globals(),
        locals);

    return plugin_manager;
}
