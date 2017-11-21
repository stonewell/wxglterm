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

    void SetWindowTitle(const std::string & title) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermWindowBase, "set_window_title", SetWindowTitle, title );
    }

    uint32_t GetColorByIndex(uint32_t index) override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermWindowBase, "get_color_by_index", GetColorByIndex, index );
    }

};
