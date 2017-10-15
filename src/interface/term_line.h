#pragma once

#include "plugin.h"

class TermLine : public virtual Plugin {
public:
    virtual void Resize(uint32_t col) = 0;

    virtual TermCellPtr GetCell(uint32_t col) = 0;
    //return the removed cell at end of line
    virtual TermCellPtr InsertCell(uint32_t col) = 0;
};
