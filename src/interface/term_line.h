#pragma once

#include "plugin.h"
#include "term_cell.h"

class TermLine : public virtual Plugin {
public:
    virtual void Init(uint32_t col) = 0;
    virtual void Resize(uint32_t col) = 0;

    virtual TermCellPtr GetCell(uint32_t col) = 0;
};

using TermLinePtr = std::shared_ptr<TermLine>;
