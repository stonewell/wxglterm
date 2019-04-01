#pragma once

#include "plugin.h"

class TermSelection : public Plugin {
public:
    virtual uint32_t GetRowBegin() const = 0;
    virtual void SetRowBegin(uint32_t rowBegin) = 0;
    virtual uint32_t GetColBegin() const = 0;
    virtual void SetColBegin(uint32_t colBegin) = 0;
    virtual uint32_t GetRowEnd() const = 0;
    virtual void SetRowEnd(uint32_t rowEnd) = 0;
    virtual uint32_t GetColEnd() const = 0;
    virtual void SetColEnd(uint32_t colEnd) = 0;
};
