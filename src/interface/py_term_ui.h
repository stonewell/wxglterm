#pragma once

#include "py_plugin.h"
#include "term_ui.h"

template<class TermUIBase = TermUI>
class PyTermUI : public virtual PyPlugin<TermUIBase> {
public:
    using PyPlugin<TermUIBase>::PyPlugin;

    int32_t StartMainUILoop() override {
        PYBIND11_OVERLOAD_PURE_NAME(int32_t, TermUIBase, "start_main_ui_loop", StartMainUILoop, );
    }

    TermWindowPtr CreateWindow() override {
        PYBIND11_OVERLOAD_PURE_NAME(TermWindowPtr, TermUIBase, "create_window", CreateWindow, );
    }
};
