#pragma once

#include "plugin_manager.h"

class PluginManagerImpl : public PluginManager {
public:
    PluginManagerImpl();
    virtual ~PluginManagerImpl();

public:
    void RegisterPlugin(Plugin * plugin) override;
    void RegisterPlugin(const char * plugin_file_path) override;
};
