#pragma once

#include "py_plugin.h"
#include "term_window.h"

template<class TermWindowBase = TermWindow>
class PyTermWindow : public virtual PyPlugin<TermWindowBase> {
public:
    using PyPlugin<TermWindowBase>::PyPlugin;

    void Refresh() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermWindowBase, "refresh", Refresh, );
    }

    void Show() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermWindowBase, "show", Show, );
    }

};
