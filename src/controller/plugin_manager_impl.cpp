#include <iostream>
#include <string.h>
#include <string>

#include <pybind11/embed.h>
namespace py = pybind11;

#include "plugin_manager_impl.h"
#include "wxglterm_interface.h"

#define FUNC_REGISTER_PLUGINS "register_plugins"

#define EXT_PYTHON ".py"

#ifdef _WIN32
#define EXT_DYLIB ".dll"
#else
#define EXT_DYLIB ".so"
#endif

PluginManagerImpl::PluginManagerImpl()
{
}

PluginManagerImpl::~PluginManagerImpl()
{
}

void PluginManagerImpl::RegisterPlugin(Plugin * plugin)
{
    print_plugin_info(plugin);
}

void PluginManagerImpl::RegisterPlugin(const char * plugin_file_path)
{
    std::cerr << "load plugin from file:"
              << plugin_file_path
              << std::endl;

    const char * ext = nullptr;

    if ((ext = strrchr(plugin_file_path, '.')))
    {
        if  (!strcmp(ext, EXT_PYTHON))
        {
            try
            {
                LoadPythonPlugin(plugin_file_path);
            }
            catch(...)
            {
                std::cerr << "!!load python plugin from file:"
                          << plugin_file_path
                          << " failed!"
                          << std::endl;
                PyErr_Print();
            }
        }
    }
}

void PluginManagerImpl::LoadPythonPlugin(const char * plugin_file_path)
{
    std::cerr << "!!load python plugin from file:"
              << plugin_file_path
              << std::endl;

    py::dict locals;
    locals["path"]        = py::cast(plugin_file_path);

    py::eval<py::eval_statements>(            // tell eval we're passing multiple statements
        "import imp\n"
        "import os\n"
        "module_name = os.path.basename(path)[:-3]\n"
        "new_module = imp.load_module(module_name, open(path), path, ('py', 'U', imp.PY_SOURCE))\n",
        py::globals(),
        locals);

    auto py_module = locals["new_module"];
    py::print(py_module);

    if (py::hasattr(py_module, FUNC_REGISTER_PLUGINS))
    {
        auto py_register_plugins_func =
                py_module.attr(FUNC_REGISTER_PLUGINS).cast<py::function>();
        py::print(py_register_plugins_func);

        py_register_plugins_func(dynamic_cast<PluginManager*>(this));
    }
}
