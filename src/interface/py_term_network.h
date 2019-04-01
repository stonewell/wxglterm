#pragma once

#include "py_multiple_instance_plugin.h"
#include "term_network.h"

template<class TermNetworkBase = TermNetwork>
class PyTermNetwork : public PyMultipleInstancePlugin<TermNetworkBase> {
public:
    using PyMultipleInstancePlugin<TermNetworkBase>::PyMultipleInstancePlugin;

public:
    void Disconnect() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermNetworkBase, "disconnect", Disconnect, );
    }

    void Connect(const char * host, int port, const char * user_name, const char * password) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermNetworkBase, "connect", Connect, host, port, user_name, password);
    }

    void Send(const std::vector<unsigned char> & data, size_t n) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermNetworkBase, "send", Send, data, n);
    }

    void Resize(uint32_t row, uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermNetworkBase, "resize", resize, row, col);
    }
};
