#include <iostream>

#include "plugin_manager_impl.h"

PluginManagerImpl::PluginManagerImpl()
{
}

PluginManagerImpl::~PluginManagerImpl()
{
}

void PluginManagerImpl::RegisterPlugin(Plugin * plugin)
{
    (void)plugin;
}

void PluginManagerImpl::RegisterPlugin(const char * plugin_file_path)
{
    std::cerr << "load plugin from file:"
              << plugin_file_path
              << std::endl;
}
