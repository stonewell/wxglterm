#pragma once

#include "py_plugin.h"
#include "term_window.h"

template<class TermWindowBase = TermWindow>
class PyTermWindow : public PyPlugin<TermWindowBase> {
public:
    using PyPlugin<TermWindowBase>::PyPlugin;

    void Refresh() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermWindowBase, "refresh", Refresh, );
    }

    void Show() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermWindowBase, "show", Show, );
    }

    void Close() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermWindowBase, "close", Close, );
    }

    void SetWindowTitle(const std::string & title) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermWindowBase, "set_window_title", SetWindowTitle, title );
    }

    uint32_t GetColorByIndex(uint32_t index) override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermWindowBase, "get_color_by_index", GetColorByIndex, index );
    }

    void SetColorByIndex(uint32_t index, uint32_t v) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermWindowBase, "set_color_by_index", SetColorByIndex, index, v );
    }

    std::string GetSelectionData() override {
        PYBIND11_OVERLOAD_PURE_NAME(std::string, TermWindowBase, "get_selection_data", GetSelectionData, );
    }

    void SetSelectionData(const std::string & sel_data) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermWindowBase, "set_selection_data", SetSelectionData, sel_data );
    }

    void EnableMouseTrack(bool enable) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermWindowBase, "enable_mouse_track", EnableMouseTrack, enable );
    }

    uint32_t GetWidth() override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermWindowBase, "get_width", GetWidth, );
    }

    uint32_t GetHeight() override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermWindowBase, "get_height", GetHeight, );
    }

    uint32_t GetLineHeight() override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermWindowBase, "get_line_height", GetLineHeight, );
    }

    uint32_t GetColWidth() override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermWindowBase, "get_col_width", GetColWidth, );
    }
};
