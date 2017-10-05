#pragma once

#include "py_context.h"
#include "term_context.h"

template<class TermContextBase = TermContext>
class PyTermContext : public virtual PyContext<TermContextBase> {
public:
    using PyContext<TermContextBase>::PyContext;

public:
    TermBufferPtr GetTermBuffer() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermBufferPtr, TermContextBase, "get_term_buffer", GetTermBuffer);
    }

    void SetTermBuffer(TermBufferPtr term_buffer) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermContextBase, "set_term_buffer", SetTermBuffer, term_buffer);
    }

    TermUIPtr GetTermUI() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermUIPtr, TermContextBase, "get_term_ui", GetTermUI);
    }
    void SetTermUI(TermUIPtr term_ui) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermContextBase, "set_term_ui", SetTermUI, term_ui);
    }

    TermNetworkPtr GetTermNetwork() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermNetworkPtr, TermContextBase, "get_term_network", GetTermNetwork);
    }

    void SetTermNetwork(TermNetworkPtr term_network) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermContextBase, "set_term_network", SetTermNetwork, term_network);
    }
};
