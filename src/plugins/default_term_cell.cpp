#include "plugin_base.h"

#include "default_term_cell.h"
#include <vector>
#include <bitset>
#include <iostream>
#include <stdio.h>
#include <iomanip>

class DefaultTermCell : public virtual PluginBase, public virtual TermCell {
public:
    DefaultTermCell(TermLine * term_line) :
        PluginBase("default_term_cell", "default terminal cell plugin", 0)
        , m_TermLine(term_line)
        , m_Char(' ')
        , m_ForeColorIdx{TermCell::DefaultForeColorIndex}
        , m_BackColorIdx{TermCell::DefaultBackColorIndex}
        , m_Mode{0}
        , m_IsWideChar(false)
        , m_Hash {.v = 0}
    {
        (void)m_TermLine;
        SetModified(false);
    }

    wchar_t GetChar() const {
        return m_Char;
    }

    void SetChar(wchar_t c) override {
        m_Char = c;
    }

    uint16_t GetForeColorIndex() const override {
        return m_ForeColorIdx;
    }
    void SetForeColorIndex(uint16_t idx) override {
        m_ForeColorIdx = idx;
    }
    uint16_t GetBackColorIndex() const override {
        return m_BackColorIdx;
    }
    void SetBackColorIndex(uint16_t idx) override {
        m_BackColorIdx = idx;
    }

    uint16_t GetMode() const override {
        return (uint16_t)m_Mode.to_ulong();
    }

    void SetMode(uint16_t m) override {
        m_Mode = std::bitset<16>(m);
    }

    void AddMode(uint16_t m) override {
        m_Mode.set(m);
    }
    void RemoveMode(uint16_t m) override {
        m_Mode.reset(m);
    }

    void Reset(TermCellPtr cell) override {
        m_Char = cell->GetChar();
        m_ForeColorIdx = cell->GetForeColorIndex();
        m_BackColorIdx = cell->GetBackColorIndex();
        m_Mode = cell->GetMode();
        m_IsWideChar = cell->IsWideChar();
    }

    bool IsWideChar() const override {
        return m_IsWideChar;
    }

    void SetWideChar(bool wide_char) override {
        m_IsWideChar = wide_char;
    }

    bool IsModified() const override {
        CellHash h;

        h.vv.c = m_Char;
        h.vv.fore = m_ForeColorIdx;
        h.vv.back = m_BackColorIdx;
        h.vv.mode = (uint16_t)m_Mode.to_ulong();
        h.vv.w = m_IsWideChar;

        return h.v != m_Hash.v;
    }

    void SetModified(bool modified) override {
        if (modified) {
            m_Hash.v = 0;
        } else {
            CellHash h {
                .vv = { m_Char,
                        m_ForeColorIdx,
                        m_BackColorIdx,
                        (uint16_t)m_Mode.to_ulong(),
                        m_IsWideChar
                }
            };

            m_Hash.v = h.v;
        }
    }
private:
#pragma pack(push , 1)
    using CellHash = union {
        uint64_t v;
        struct {
            wchar_t c;
            uint16_t fore:9;
            uint16_t back:9;
            uint16_t mode:13;
            bool w:1;
        } vv;
    };
#pragma pack(pop)

    TermLine * m_TermLine;

    wchar_t m_Char;
    uint16_t m_ForeColorIdx;
    uint16_t m_BackColorIdx;
    std::bitset<16> m_Mode;
    bool m_IsWideChar;
    CellHash m_Hash;
};

TermCellPtr CreateDefaultTermCell(TermLine * line)
{
    return TermCellPtr { new DefaultTermCell(line) };
}
