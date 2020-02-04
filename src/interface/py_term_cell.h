#pragma once

#include "py_plugin.h"
#include "term_cell.h"

template<class TermCellBase = TermCell>
class PyTermCell : public PyPlugin<TermCellBase> {
public:
    using PyPlugin<TermCellBase>::PyPlugin;

    wchar_t GetChar() const override {
        PYBIND11_OVERLOAD_PURE_NAME(wchar_t, TermCellBase, "get_char", GetChar, );
    }
    void SetChar(wchar_t c) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "set_char", SetChar, c);
    }

    uint32_t GetForeColorIndex() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermCellBase, "get_fore_color_idx", GetForeColorIndex, );
    }
    void SetForeColorIndex(uint32_t idx) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "set_fore_color_idx", SetForeColorIndex, idx);
    }
    uint32_t GetBackColorIndex() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermCellBase, "get_back_color_idx", GetBackColorIndex, );
    }
    void SetBackColorIndex(uint32_t idx) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "set_back_color_idx", SetBackColorIndex, idx);
    }

    uint32_t GetMode() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermCellBase, "get_mode", GetMode, );
    }
    void SetMode(uint32_t m) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "set_mode", SetMode, m);
    }
    void AddMode(uint32_t m) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "add_mode", AddMode, m);
    }
    void RemoveMode(uint32_t m) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "remove_mode", RemoveMode, m);
    }

    void Reset(const TermCellPtr & cell) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "reset", Reset, cell);
    }

    bool IsWideChar() const override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, TermCellBase, "is_wide_char", IsWideChar,);
    }

    void SetWideChar(bool wide_char) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "set_wide_char", SetWideChar, wide_char);
    }

    bool IsModified() const override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, TermCellBase, "is_modified", IsModified, );
    }

    void SetModified(bool modified) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermCellBase, "set_modified", SetModified, modified);
    }
};
