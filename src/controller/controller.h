#pragma once

#include "plugin_manager.h"
#include <memory>

std::shared_ptr<PluginManager> LoadAllPlugins(const char * plugin_path);
