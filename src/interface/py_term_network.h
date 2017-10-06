#pragma once

#include "py_multiple_instance_plugin.h"
#include "term_network.h"

template<class TermNetworkBase = TermNetwork>
class PyTermNetwork : public virtual PyMultipleInstancePlugin<TermNetworkBase> {
public:
    using PyMultipleInstancePlugin<TermNetworkBase>::PyMultipleInstancePlugin;

public:
    void Disconnect() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermNetworkBase, "disconnect", Disconnect, );
    }

    void Connect(const char * host, int port, const char * user_name, const char * password) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermNetworkBase, "connect", Connect, host, port, user_name, password);
    }

};
