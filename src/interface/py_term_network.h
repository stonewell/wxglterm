#pragma once

#include "py_plugin.h"
#include "term_network.h"

template<class TermNetworkBase = TermNetwork>
class PyTermNetwork : public virtual PyPlugin<TermNetworkBase> {
public:
    using PyPlugin<TermNetworkBase>::PyPlugin;

public:
    void Disconnect() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermNetworkBase, "disconnect", Disconnect, );
    }
};
