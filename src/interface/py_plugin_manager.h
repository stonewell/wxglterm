#pragma once

#include "plugin_manager.h"

template<class PluginManagerBase = PluginManager>
class PyPluginManager : public PluginManagerBase {
public:
    using PluginManagerBase::PluginManagerBase;

public:
    void RegisterPlugin(Plugin * plugin) override {
        PYBIND11_OVERLOAD_PURE_NAME(void,
                                    PluginManagerBase,
                                    "register_plugin",
                                    RegisterPlugin,
                                    plugin);
    }

    void RegisterPlugin(const char * plugin_file_path) override {
        PYBIND11_OVERLOAD_PURE_NAME(void,
                                    PluginManagerBase,
                                    "register_plugin",
                                    RegisterPlugin,
                                    plugin_file_path);
    }
};
