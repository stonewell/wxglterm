#pragma once

#include "py_context.h"
#include "term_context.h"

template<class TermContextBase = TermContext>
class PyTermContext : public PyContext<TermContextBase> {
public:
    using TermContextBase::TermContextBase;

public:
    TermBuffer * GetTermBuffer() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermBuffer *, TermContextBase, "get_term_buffer", GetTermBuffer);
    }
    TermUI * GetTermUI() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermUI *, TermContextBase, "get_term_ui", GetTermUI);
    }
    TermNetwork * GetTermNetwork() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermNetwork *, TermContextBase, "get_term_network", GetTermNetwork);
    }
};
