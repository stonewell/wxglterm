#include "plugin_base.h"

#include "default_term_cell.h"
#include <vector>
#include <bitset>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <string.h>

class DefaultTermCell : public TermCell {
public:
    DefaultTermCell(TermLine * term_line) :
        PLUGIN_BASE_INIT_LIST("default_term_cell", "default terminal cell plugin", 0)
        , m_TermLine(term_line)
        , m_Char(' ')
        , m_ForeColorIdx{TermCell::DefaultForeColorIndex}
        , m_BackColorIdx{TermCell::DefaultBackColorIndex}
        , m_Mode{0}
        , m_IsWideChar(false)
        , m_Hash {}
    {
        (void)m_TermLine;
        SetModified(false);
    }

	PLUGIN_BASE_DEFINE();

    wchar_t GetChar() const {
        return m_Char;
    }

    void SetChar(wchar_t c) override {
        m_Char = c;
    }

    uint32_t GetForeColorIndex() const override {
        return m_ForeColorIdx;
    }
    void SetForeColorIndex(uint32_t idx) override {
        m_ForeColorIdx = idx;
    }
    uint32_t GetBackColorIndex() const override {
        return m_BackColorIdx;
    }
    void SetBackColorIndex(uint32_t idx) override {
        m_BackColorIdx = idx;
    }

    uint32_t GetMode() const override {
        return (uint32_t)m_Mode.to_ulong();
    }

    void SetMode(uint32_t m) override {
        m_Mode = std::bitset<16>(m);
    }

    void AddMode(uint32_t m) override {
        m_Mode.set(m);
    }
    void RemoveMode(uint32_t m) override {
        m_Mode.reset(m);
    }

    void Reset(const TermCellPtr & cell) override {
        m_Char = cell->GetChar();
        m_ForeColorIdx = cell->GetForeColorIndex();
        m_BackColorIdx = cell->GetBackColorIndex();
        m_Mode = std::bitset<16>(cell->GetMode());
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
        h.vv.mode = m_Mode.to_ulong();
        h.vv.w = m_IsWideChar;

        return !(h == m_Hash);
    }

    void SetModified(bool modified) override {
        if (modified) {
            m_Hash.Clear();
        } else {
            m_Hash.vv.c = m_Char;
            m_Hash.vv.fore = m_ForeColorIdx;
            m_Hash.vv.back = m_BackColorIdx;
            m_Hash.vv.mode = m_Mode.to_ulong();
            m_Hash.vv.w = m_IsWideChar;
        }
    }
private:
#pragma pack(push , 1)
    using CellHash = union __CellHash {
        uint32_t v[5];
        struct {
            wchar_t c;
            uint32_t fore;
            uint32_t back;
            uint32_t mode;
            bool w:1;
        } vv;

        __CellHash() {
            Clear();
        }

        __CellHash & operator = (const __CellHash & u) {
            memmove(v, u.v, sizeof(v));
            return *this;
        }

        void Clear() {
            memset(v, 0, sizeof(v));
        }

        bool operator == (const __CellHash & u) {
            return memcmp(v, u.v, sizeof(v)) == 0;
        }
    };
#pragma pack(pop)

    TermLine * m_TermLine;

    wchar_t m_Char;
    uint32_t m_ForeColorIdx;
    uint32_t m_BackColorIdx;
    std::bitset<16> m_Mode;
    bool m_IsWideChar;
    CellHash m_Hash;
};

TermCellPtr CreateDefaultTermCell(TermLine * line)
{
    return TermCellPtr { new DefaultTermCell(line) };
}
