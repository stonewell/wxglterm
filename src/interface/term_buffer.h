#pragma once

#include "multiple_instance_plugin.h"
#include "term_line.h"
#include "term_cell.h"

class TermBuffer : public virtual MultipleInstancePlugin {
public:
    virtual void Init(uint32_t row, uint32_t col) = 0;
    virtual void Resize(uint32_t row, uint32_t col) = 0;

    virtual uint32_t GetRows() const = 0;
    virtual uint32_t GetCols() const = 0;
    virtual uint32_t GetRow() const = 0;
    virtual uint32_t GetCol() const = 0;

    virtual TermLinePtr GetLine(uint32_t row) = 0;
    virtual TermCellPtr GetCell(uint32_t row, uint32_t col) = 0;
    virtual TermLinePtr GetCurLine() = 0;
    virtual TermCellPtr GetCurCell() = 0;
};

using TermBufferPtr = std::shared_ptr<TermBuffer>;
