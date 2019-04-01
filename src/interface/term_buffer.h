#pragma once

#include "multiple_instance_plugin.h"
#include "term_line.h"
#include "term_cell.h"

class TermBuffer : public MultipleInstancePlugin {
public:
    virtual void Resize(uint32_t row, uint32_t col) = 0;

    virtual uint32_t GetRows()= 0;
    virtual uint32_t GetCols()= 0;

    virtual uint32_t GetRow()= 0;
    virtual void SetRow(uint32_t row) = 0;
    virtual uint32_t GetCol()= 0;
    virtual void SetCol(uint32_t col) = 0;

    virtual TermLinePtr GetLine(uint32_t row) = 0;
    virtual TermCellPtr GetCell(uint32_t row, uint32_t col) = 0;
    virtual TermLinePtr GetCurLine() = 0;
    virtual TermCellPtr GetCurCell() = 0;

    virtual uint32_t GetScrollRegionBegin()= 0;
    virtual uint32_t GetScrollRegionEnd()= 0;
    virtual void SetScrollRegionBegin(uint32_t begin) = 0;
    virtual void SetScrollRegionEnd(uint32_t end) = 0;

    virtual void ScrollBuffer(int32_t offset, TermCellPtr cell_template) = 0;
    virtual void DeleteLines(uint32_t begin, uint32_t count, TermCellPtr cell_template) = 0;
    virtual void InsertLines(uint32_t begin, uint32_t count, TermCellPtr cell_template) = 0;
    virtual bool MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer, TermCellPtr cell_template) = 0;

    virtual void SetCellDefaults(wchar_t c,
                                 uint32_t fore_color_idx,
                                 uint32_t back_color_idx,
                                 uint32_t mode) = 0;
    virtual TermCellPtr CreateCellWithDefaults() = 0;

    virtual void SetSelection(TermSelectionPtr selection) = 0;
    virtual TermSelectionPtr GetSelection() = 0;
    virtual void ClearSelection() = 0;

    virtual void SetCurCellData(uint32_t ch, bool wide_char, bool insert, TermCellPtr cell_template) = 0;

    virtual void LockUpdate() = 0;
    virtual void UnlockUpdate() = 0;

    virtual void EnableAlterBuffer(bool enable) = 0;

    virtual TermBufferPtr CloneBuffer() = 0;

    virtual uint32_t GetMode() = 0;
    virtual void SetMode(uint32_t m) = 0;
    virtual void AddMode(uint32_t m) = 0;
    virtual void RemoveMode(uint32_t m) = 0;

    virtual void SetProperty(const std::string & key, const std::string & v) = 0;
    virtual const std::string & GetProperty(const std::string & key) = 0;
};
