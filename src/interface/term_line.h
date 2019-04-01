#pragma once

#include "plugin.h"

class TermLine : public Plugin {
public:
    virtual void Resize(uint32_t col) = 0;
    virtual void Resize(uint32_t col, TermCellPtr cell_template) = 0;

    virtual TermCellPtr GetCell(uint32_t col) = 0;
    //return the removed cell at end of line
    virtual TermCellPtr InsertCell(uint32_t col) = 0;
    virtual bool IsModified() const = 0;
    virtual void SetModified(bool modified) = 0;
    virtual uint32_t GetLastRenderLineIndex() const = 0;
    virtual void SetLastRenderLineIndex(uint32_t index) = 0;
};
