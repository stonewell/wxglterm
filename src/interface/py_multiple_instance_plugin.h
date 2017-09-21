#pragma once

#include "multiple_instance_plugin.h"
#include "py_plugin.h"

template<class MultipleInstancePluginBase = MultipleInstancePlugin>
class PyMultipleInstancePlugin : public PyPlugin<MultipleInstancePluginBase> {
public:
    using MultipleInstancePluginBase::MultipleInstancePluginBase;

public:
    MultipleInstancePluginBase * NewInstance() override {
        PYBIND11_OVERLOAD_PURE_NAME(MultipleInstancePluginBase *, MultipleInstancePluginBase, "new_instance", NewInstance, );
    }
};
