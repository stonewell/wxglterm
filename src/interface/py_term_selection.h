#pragma once

#include "term_selection.h"
#include "py_multiple_instance_plugin.h"

template<class TermSelectionBase = TermSelection>
class PyTermSelection : public PyPlugin<TermSelectionBase> {
public:
    using PyPlugin<TermSelectionBase>::PyPlugin;

    uint32_t GetRowBegin() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermSelectionBase, "get_row_begin", GetRowBegin, );
    }
    void SetRowBegin(uint32_t rowBegin) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermSelectionBase, "set_row_begin", SetRowBegin, rowBegin);
    }
    uint32_t GetColBegin() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermSelectionBase, "get_col_begin", GetColBegin, );
    }
    void SetColBegin(uint32_t colBegin) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermSelectionBase, "set_col_begin", SetColBegin, colBegin);
    }
    uint32_t GetRowEnd() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermSelectionBase, "get_row_end", GetRowEnd, );
    }
    void SetRowEnd(uint32_t rowEnd) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermSelectionBase, "set_row_end", SetRowEnd, rowEnd);
    }
    uint32_t GetColEnd() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermSelectionBase, "get_col_end", GetColEnd, );
    }
    void SetColEnd(uint32_t colEnd) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermSelectionBase, "set_col_end", SetColEnd, colEnd);
    }
};
