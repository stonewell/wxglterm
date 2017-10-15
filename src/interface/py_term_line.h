#pragma once

#include "term_line.h"
#include "py_plugin.h"

template<class TermLineBase = TermLine>
class PyTermLine : public virtual PyPlugin<TermLineBase> {
public:
    using PyPlugin<TermLineBase>::PyPlugin;

    void Resize(uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermLineBase, "resize", Resize, col);
    }

    TermCellPtr GetCell(uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(TermCellPtr, TermLineBase, "get_cell", GetCell, col);
    }

    TermCellPtr InsertCell(uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(TermCellPtr, TermLineBase, "insert_cell", InsertCell, col);
    }
};
