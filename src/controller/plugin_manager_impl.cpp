#include <iostream>
#include <string>
#include <exception>

#include <string.h>

#include <pybind11/embed.h>
namespace py = pybind11;

#include "plugin_manager_impl.h"
#include "wxglterm_interface.h"
#include "load_module.h"
#include "plugin.h"

#define FUNC_REGISTER_PLUGINS "register_plugins"

#define EXT_PYTHON ".py"

#ifdef _WIN32
#define EXT_DYLIB ".dll"
#elif defined(__APPLE__)
#define EXT_DYLIB ".dylib"
#else
#define EXT_DYLIB ".so"
#endif

extern bool g_AppDebug;

static
void insert_into_forward_list(PluginList & plugin_list, SharedPluginPtr plugin)
{
    PluginList::iterator it, it_end, it_before;

    for(it = plugin_list.begin(),
                it_end = plugin_list.end(),
                it_before = plugin_list.before_begin();
        it != it_end;
        it_before = it, it++)
    {
        if ((*it)->GetVersion() < plugin->GetVersion())
        {
            plugin_list.insert_after(it_before, plugin);
            return;
        }
    }

    plugin_list.insert_after(it_before, plugin);
}

PluginManagerImpl::PluginManagerImpl()
{
}

PluginManagerImpl::~PluginManagerImpl()
{
}

void PluginManagerImpl::RegisterPlugin(SharedPluginPtr plugin)
{
    print_plugin_info(plugin.get());

    PluginMap::iterator it = m_PluginMap.find(plugin->GetName());

    if (it != m_PluginMap.end())
    {
        insert_into_forward_list(it->second, plugin);
    }
    else
    {
        PluginList pluginList;

        pluginList.push_front(plugin);

        m_PluginMap.emplace(std::string(plugin->GetName()),
                            pluginList);
    }
}

void PluginManagerImpl::RegisterPlugin(const char * plugin_file_path)
{
    if (g_AppDebug)
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
        else if (!strcmp(ext, EXT_DYLIB))
        {
            LoadDylibPlugin(plugin_file_path);
        }
    }
}

void PluginManagerImpl::LoadPythonPlugin(const char * plugin_file_path)
{
    if (g_AppDebug)
        std::cerr << "!!load python plugin from file:"
                  << plugin_file_path
                  << std::endl;

    auto py_module = LoadPyModuleFromFile(plugin_file_path);
    py::print(py_module);

    if (py::hasattr(py_module, FUNC_REGISTER_PLUGINS))
    {
        auto py_register_plugins_func =
                py_module.attr(FUNC_REGISTER_PLUGINS).cast<py::function>();
        py::print(py_register_plugins_func);

        py_register_plugins_func(std::dynamic_pointer_cast<PluginManager>(shared_from_this()));
    }
}

SharedPluginPtr PluginManagerImpl::GetPlugin(const char * plugin_name, uint64_t plugin_version_code)
{
    if (g_AppDebug) {
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
    }

    PluginMap::iterator it = m_PluginMap.find(plugin_name);

    if (it == m_PluginMap.end())
        return SharedPluginPtr {};

    if (plugin_version_code == static_cast<uint64_t>(PluginManager::Latest))
    {
        return it->second.front();
    }

    for(auto plugin : it->second)
    {
        if (plugin->GetVersion() == plugin_version_code)
            return plugin;
    }

    if (g_AppDebug)
        std::cerr << "get plugin:"
                  << plugin_name
                  << ", with version:"
                  << plugin_version_code
                  << " not found, return latest"
                  << std::endl;

    return it->second.front();
}

void PluginManagerImpl::LoadDylibPlugin(const char * plugin_file_path)
{
    if (g_AppDebug)
        std::cerr << "!!load dylib plugin from file:"
                  << plugin_file_path
                  << std::endl;

    auto h = LoadDyModuleFromFile(plugin_file_path);

    if (!h)
    {
        return;
    }

    typedef void (*func_register_plugins_t)(std::shared_ptr<PluginManager>);

    func_register_plugins_t func_register_plugin =
            (func_register_plugins_t)GetDyProcAddress(h,
                                                      FUNC_REGISTER_PLUGINS);

    if (!func_register_plugin)
        return;

    func_register_plugin(std::dynamic_pointer_cast<PluginManager>(shared_from_this()));

    m_DylibHandleList.push_front(h);

    if (g_AppDebug)
        std::cerr << "!!load dylib plugin from file:"
                  << plugin_file_path
                  << " done."
                  << std::endl;
}
