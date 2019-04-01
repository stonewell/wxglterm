#pragma once

#include "multiple_instance_plugin.h"
#include "py_plugin.h"

template<class MultipleInstancePluginBase = MultipleInstancePlugin>
class PyMultipleInstancePlugin : public PyPlugin<MultipleInstancePluginBase> {
public:
    using PyPlugin<MultipleInstancePluginBase>::PyPlugin;

public:
    MultipleInstancePluginPtr NewInstance() override {
        PYBIND11_OVERLOAD_PURE_NAME(MultipleInstancePluginPtr,
                                    MultipleInstancePluginBase,
                                    "new_instance",
                                    NewInstance, );
    }
};
