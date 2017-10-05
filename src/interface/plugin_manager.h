#pragma once

#include "plugin.h"

class PluginManager {
public:
    enum VersionCodeEnum {
        Latest = -1,
    };

    PluginManager() = default;
    virtual ~PluginManager() = default;

public:
    virtual void RegisterPlugin(PluginPtr plugin) = 0;
    virtual void RegisterPlugin(const char * plugin_file_path) = 0;
    virtual PluginPtr GetPlugin(const char * plugin_name, uint64_t plugin_version_code) = 0;
};
