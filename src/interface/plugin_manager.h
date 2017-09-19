#pragma once

class Plugin;

class PluginManager {
public:
    PluginManager() = default;
    virtual ~PluginManager() = default;

public:
    virtual void RegisterPlugin(Plugin * plugin) = 0;
    virtual void RegisterPlugin(const char * plugin_file_path) = 0;
};
