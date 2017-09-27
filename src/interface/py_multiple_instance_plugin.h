#pragma once

#include "multiple_instance_plugin.h"
#include "py_plugin.h"

template<class MultipleInstancePluginBase = MultipleInstancePlugin>
class PyMultipleInstancePlugin : public virtual PyPlugin<MultipleInstancePluginBase> {
public:
    using PyPlugin<MultipleInstancePluginBase>::PyPlugin;

public:
    std::shared_ptr<MultipleInstancePlugin> NewInstance() override {
        PYBIND11_OVERLOAD_PURE_NAME(std::shared_ptr<MultipleInstancePlugin>, MultipleInstancePluginBase, "new_instance", NewInstance, );
    }
};
