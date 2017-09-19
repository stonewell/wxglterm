#pragma once

#include "plugin.h"

template<class PluginBase = Plugin>
class PyPlugin : public PluginBase {
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
};
