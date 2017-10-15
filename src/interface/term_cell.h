#pragma once

#include "plugin.h"

class TermCell : public virtual Plugin {
public:
    enum ColorIndexEnum {
        DefaultColorIndex = 256,
    };

    enum TextModeEnum {
        Stdout = 0,
        Reverse = 1,
        Selection = 2,
        Cursor = 3,
        Bold = 4,
        Dim = 5,
    };

    virtual wchar_t GetChar() const = 0;
    virtual void SetChar(wchar_t c) = 0;

    virtual uint16_t GetForeColorIndex() const = 0;
    virtual void SetForeColorIndex(uint16_t idx) = 0;
    virtual uint16_t GetBackColorIndex() const = 0;
    virtual void SetBackColorIndex(uint16_t idx) = 0;

    virtual uint16_t GetMode() const = 0;
    virtual void SetMode(uint16_t m) = 0;
    virtual void AddMode(uint16_t m) = 0;
    virtual void RemoveMode(uint16_t m) = 0;

    virtual void Reset(TermCellPtr cell) = 0;

    virtual bool IsWideChar() const = 0;
    virtual void SetWideChar(bool wide_char) = 0;
};
