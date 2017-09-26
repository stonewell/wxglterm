#include <iostream>
#include <string>
#include <exception>

#include <string.h>

#include <pybind11/embed.h>
namespace py = pybind11;

#include "plugin_manager_impl.h"
#include "wxglterm_interface.h"
#include "load_module.h"

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
            catch(std::exception & e)
            {
                std::cerr << "!!load python plugin from file:"
                          << plugin_file_path
                          << " failed!"
                          << std::endl
                          << e.what()
                          << std::endl;
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

    auto py_module = LoadModuleFromFile(plugin_file_path);
    py::print(py_module);

    if (py::hasattr(py_module, FUNC_REGISTER_PLUGINS))
    {
        auto py_register_plugins_func =
                py_module.attr(FUNC_REGISTER_PLUGINS).cast<py::function>();
        py::print(py_register_plugins_func);

        py_register_plugins_func(dynamic_cast<PluginManager*>(this));
    }
}

Plugin * PluginManagerImpl::GetPlugin(const char * plugin_name, uint64_t plugin_version_code)
{
    std::cerr << "get plugin:"
              << plugin_name
              << ", with version:";
    if (plugin_version_code == static_cast<uint64_t>(PluginManager::Latest))
    {
        std::cerr << "Latest";
    }
    else
        std::cerr << plugin_version_code;

    std::cerr << std::endl;
    return nullptr;
}
