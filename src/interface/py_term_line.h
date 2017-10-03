#pragma once

#include "term_line.h"
#include "py_plugin.h"

template<class TermLineBase = TermLine>
class PyTermLine : public virtual PyPlugin<TermLineBase> {
public:
    using PyPlugin<TermLineBase>::PyPlugin;

    void Init(uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermLineBase, "init", Init, col);
    }

    void Resize(uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermLineBase, "resize", Resize, col);
    }

    TermCellPtr GetCell(uint32_t col) {
        PYBIND11_OVERLOAD_PURE_NAME(TermCellPtr, TermLineBase, "get_cell", GetCell, col);
    }
};
