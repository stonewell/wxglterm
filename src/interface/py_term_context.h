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

    TermColorThemePtr GetTermColorTheme() const override {
        PYBIND11_OVERLOAD_PURE_NAME(TermColorThemePtr, TermContextBase, "get_term_color_theme", GetTermColorTheme, );
    }

    void SetTermColorTheme(TermColorThemePtr term_color_theme) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermContextBase, "set_term_color_theme", SetTermColorTheme, term_color_theme);
    }
    InputHandlerPtr GetInputHandler() const override {
        PYBIND11_OVERLOAD_PURE_NAME(InputHandlerPtr, TermContextBase, "get_input_handler", GetInputHandler, );
    }

    void SetInputHandler(InputHandlerPtr input_plugin) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermContextBase, "set_input_handler", SetInputHandler, input_plugin);
    }
};
