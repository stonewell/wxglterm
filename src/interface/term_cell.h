#pragma once

#include "plugin.h"

class TermCell : public Plugin {
public:
    enum ColorIndexEnum {
        DefaultForeColorIndex = 256,
        DefaultBackColorIndex,
        DefaultCursorColorIndex,
        ColorIndexCount,
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

    virtual uint32_t GetForeColorIndex() const = 0;
    virtual void SetForeColorIndex(uint32_t idx) = 0;
    virtual uint32_t GetBackColorIndex() const = 0;
    virtual void SetBackColorIndex(uint32_t idx) = 0;

    virtual uint32_t GetMode() const = 0;
    virtual void SetMode(uint32_t m) = 0;
    virtual void AddMode(uint32_t m) = 0;
    virtual void RemoveMode(uint32_t m) = 0;

    virtual void Reset(const TermCellPtr & cell) = 0;

    virtual bool IsWideChar() const = 0;
    virtual void SetWideChar(bool wide_char) = 0;

    virtual bool IsModified() const = 0;
    virtual void SetModified(bool modified) = 0;
};
