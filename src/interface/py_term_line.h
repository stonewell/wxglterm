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

    void Resize(uint32_t col, TermCellPtr cell_template) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermLineBase, "resize", Resize, col, cell_template);
    }

    TermCellPtr GetCell(uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(TermCellPtr, TermLineBase, "get_cell", GetCell, col);
    }

    TermCellPtr InsertCell(uint32_t col) override {
        PYBIND11_OVERLOAD_PURE_NAME(TermCellPtr, TermLineBase, "insert_cell", InsertCell, col);
    }

    bool IsModified() const override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, TermLineBase, "is_modified", IsModified, );
    }

    void SetModified(bool modified) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermLineBase, "set_modified", SetModified, modified);
    }

    uint32_t GetLastRenderLineIndex() const override {
        PYBIND11_OVERLOAD_PURE_NAME(uint32_t, TermLineBase, "get_last_render_line_index", GetLastRenderLineIndex, );
    }

    void SetLastRenderLineIndex(uint32_t index) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermLineBase, "set_last_render_line_index", SetLastRenderLineIndex, index);
    }
};
