#pragma once

#include "term_buffer.h"
#include "py_multiple_instance_plugin.h"

template<class TermBufferBase = TermBuffer>
class PyTermBuffer : public virtual PyMultipleInstancePlugin<TermBufferBase> {
public:
    using PyMultipleInstancePlugin<TermBufferBase>::PyMultipleInstancePlugin;

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
    void SetRow(uint32_t row) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "set_row", SetRow, row);
    }
    void SetCol(uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "set_col", SetCol, col);
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
    uint32_t GetScrollRegionBegin() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermBufferBase, "get_scroll_region_begin", GetScrollRegionBegin,);
    }
    uint32_t GetScrollRegionEnd() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermBufferBase, "get_scroll_region_end", GetScrollRegionEnd,);
    }
    void SetScrollRegionBegin(uint32_t begin) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "set_scroll_region_begin", SetScrollRegionBegin, begin);
    }
    void SetScrollRegionEnd(uint32_t end) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "set_scroll_region_end", SetScrollRegionEnd, end);
    }
    void ScrollBuffer(int32_t scroll_offset) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "scroll_buffer", ScrollBuffer, scroll_offset);
    }
    void DeleteLines(uint32_t begin, uint32_t count) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "delete_lines", ScrollBuffer, begin, count);
    }
    void InsertLines(uint32_t begin, uint32_t count) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "insert_lines", ScrollBuffer, begin, count);
    }
    void SetCellDefaults(wchar_t c,
                         uint16_t fore_color_idx,
                         uint16_t back_color_idx,
                         uint16_t mode) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "set_cell_defaults", SetCellDefaults, c, fore_color_idx, back_color_idx, mode);
    }

    TermCellPtr CreateCellWithDefaults() override {
        PYBIND11_OVERLOAD_PURE_NAME(TermCellPtr, TermBufferBase, "create_cell_with_defaults", CreateCellWithDefaults, );
    }

    void SetSelection(TermSelectionPtr selection) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "set_selection", SetSelection, selection );
    }

    TermSelectionPtr GetSelection() override {
        PYBIND11_OVERLOAD_PURE_NAME(TermSelectionPtr, TermBufferBase, "get_selection", GetSelection, );
    }

    void ClearSelection() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermBufferBase, "clear_selection", ClearSelection, );
    }
};
