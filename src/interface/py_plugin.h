#pragma once

#include "plugin.h"
#include "context.h"

template<class PluginBase = Plugin>
class PyPlugin : public virtual PluginBase {
public:
    using PluginBase::PluginBase;

public:
    const char * GetName() override {
        PYBIND11_OVERLOAD_PURE_NAME(const char *, PluginBase, "get_name", GetName, );
    }
    const char * GetDescription() override {
        PYBIND11_OVERLOAD_PURE_NAME(const char *, PluginBase, "get_description", GetDescription, );
    }
    uint32_t GetVersion() override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, PluginBase, "get_version", GetVersion, );
    }
    void InitPlugin(ContextPtr context,
                    AppConfigPtr plugin_config) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, PluginBase, "init_plugin", InitPlugin, context, plugin_config);
    }
    ContextPtr GetPluginContext() const override {
        PYBIND11_OVERLOAD_PURE_NAME(ContextPtr, PluginBase, "get_plugin_context", GetPluginContext, );
    }
    AppConfigPtr GetPluginConfig() const override {
        PYBIND11_OVERLOAD_PURE_NAME(AppConfigPtr, PluginBase, "get_plugin_config", GetPluginConfig, );
    }
};
