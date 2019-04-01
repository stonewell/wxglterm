#pragma once

#include "py_multiple_instance_plugin.h"
#include "plugin_context.h"

template<class ContextBase = Context>
class PyPluginContext : public PyMultipleInstancePlugin<ContextBase> {
public:
    using PyMultipleInstancePlugin<ContextBase>::PyMultipleInstancePlugin;

public:
    AppConfigPtr GetAppConfig() const override {
        PYBIND11_OVERLOAD_PURE_NAME(AppConfigPtr, ContextBase, "get_app_config", GetAppConfig, );
    }

    void SetAppConfig(AppConfigPtr app_config) override{
        PYBIND11_OVERLOAD_PURE_NAME(void, ContextBase, "set_app_config", SetAppConfig, app_config);
    }

};
