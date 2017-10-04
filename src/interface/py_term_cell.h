#pragma once

#include "py_plugin.h"
#include "term_cell.h"

template<class TermCellBase = TermCell>
class PyTermCell : public virtual PyPlugin<TermCellBase> {
public:
    using PyPlugin<TermCellBase>::PyPlugin;

    wchar_t GetChar() const override {
        PYBIND11_OVERLOAD_PURE_NAME(wchar_t, TermCellBase, "get_char", GetChar, );
    }
    void SetChar(wchar_t c) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "set_char", SetChar, c);
    }

    uint16_t GetForeColorIndex() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint16_t, TermCellBase, "get_fore_color_idx", GetForeColorIndex, );
    }
    void SetForeColorIndex(uint16_t idx) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "set_fore_color_idx", SetForeColorIndex, idx);
    }
    uint16_t GetBackColorIndex() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint16_t, TermCellBase, "get_back_color_idx", GetBackColorIndex, );
    }
    void SetBackColorIndex(uint16_t idx) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "set_back_color_idx", SetBackColorIndex, idx);
    }

    uint16_t GetMode() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint16_t, TermCellBase, "get_mode", GetMode, );
    }
    void SetMode(uint16_t m) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "set_mode", SetMode, m);
    }
};
