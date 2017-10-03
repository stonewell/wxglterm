#pragma once

#include "term_buffer.h"
#include "py_multiple_instance_plugin.h"

template<class TermBufferBase = TermBuffer>
class PyTermBuffer : public virtual PyMultipleInstancePlugin<TermBufferBase> {
public:
    using PyMultipleInstancePlugin<TermBufferBase>::PyMultipleInstancePlugin;

    void Init(uint32_t row, uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "init", Init, row, col);
    }

    void Resize(uint32_t row, uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "resize", resize, row, col);
    }

    uint32_t GetRows() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermBufferBase, "get_rows", GetRows, );
    }
    uint32_t GetCols() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermBufferBase, "get_cols", GetCols, );
    }
    uint32_t GetRow() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermBufferBase, "get_row", GetRow, );
    }
    uint32_t GetCol() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermBufferBase, "get_col", GetCol, );
    }

    TermLinePtr GetLine(uint32_t row) override {
        PYBIND11_OVERLOAD_PURE_NAME(TermLinePtr, TermBufferBase, "get_line", GetLine, row);
    }
    TermCellPtr GetCell(uint32_t row, uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(TermCellPtr, TermBufferBase, "get_cell", GetCell, row, col);
    }
    TermLinePtr GetCurLine() override {
        PYBIND11_OVERLOAD_PURE_NAME(TermLinePtr, TermBufferBase, "get_cur_line", GetCurLine,);
    }
    TermCellPtr GetCurCell() override {
        PYBIND11_OVERLOAD_PURE_NAME(TermCellPtr, TermBufferBase, "get_cur_cell", GetCurCell,);
    }
};
