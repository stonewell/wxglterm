#pragma once

#include "plugin_manager.h"
#include <memory>

std::shared_ptr<PluginManager> LoadAllPlugins(const char * plugin_path);
void LoadAllPlugins(std::shared_ptr<PluginManager> plugin_manager, const char * plugin_path);
