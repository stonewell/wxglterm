#pragma once

#include "py_context.h"
#include "term_context.h"

template<class TermContextBase = TermContext>
class PyTermContext : public virtual PyContext<TermContextBase> {
public:
    using PyContext<TermContextBase>::PyContext;

public:
    std::shared_ptr<TermBuffer> GetTermBuffer() const override {
        PYBIND11_OVERLOAD_PURE_NAME(std::shared_ptr<TermBuffer>, TermContextBase, "get_term_buffer", GetTermBuffer);
    }
    std::shared_ptr<TermUI> GetTermUI() const override {
        PYBIND11_OVERLOAD_PURE_NAME(std::shared_ptr<TermUI>, TermContextBase, "get_term_ui", GetTermUI);
    }
    std::shared_ptr<TermNetwork> GetTermNetwork() const override {
        PYBIND11_OVERLOAD_PURE_NAME(std::shared_ptr<TermNetwork>, TermContextBase, "get_term_network", GetTermNetwork);
    }
};
