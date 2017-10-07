#pragma once

#include "py_context.h"
#include "term_context.h"

template<class TermContextBase = TermContext>
class PyTermContext : public virtual PyContext<TermContextBase> {
public:
    using PyContext<TermContextBase>::PyContext;

public:
    TermBufferPtr GetTermBuffer() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermBufferPtr, TermContextBase, "get_term_buffer", GetTermBuffer, );
    }

    void SetTermBuffer(TermBufferPtr term_buffer) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermContextBase, "set_term_buffer", SetTermBuffer, term_buffer);
    }

    TermWindowPtr GetTermWindow() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermWindowPtr, TermContextBase, "get_term_window", GetTermWindow, );
    }
    void SetTermWindow(TermWindowPtr term_window) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermContextBase, "set_term_window", SetTermWindow, term_window);
    }

    TermNetworkPtr GetTermNetwork() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermNetworkPtr, TermContextBase, "get_term_network", GetTermNetwork, );
    }

    void SetTermNetwork(TermNetworkPtr term_network) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermContextBase, "set_term_network", SetTermNetwork, term_network);
    }

    TermDataHandlerPtr GetTermDataHandler() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermDataHandlerPtr, TermContextBase, "get_term_data_handler", GetTermDataHandler, );
    }

    void SetTermDataHandler(TermDataHandlerPtr term_data_handler) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermContextBase, "set_term_data_handler", SetTermDataHandler, term_data_handler);
    }
};
