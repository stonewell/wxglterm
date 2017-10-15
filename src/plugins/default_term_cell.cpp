#include "plugin_base.h"

#include "default_term_cell.h"
#include <vector>
#include <bitset>

class DefaultTermCell : public virtual PluginBase, public virtual TermCell {
public:
    DefaultTermCell(TermLine * term_line) :
        PluginBase("default_term_cell", "default terminal cell plugin", 0)
        , m_TermLine(term_line)
        , m_Mode{0}
        , m_IsWideChar(false)
    {
        (void)m_TermLine;
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
private:
    TermLine * m_TermLine;

    wchar_t m_Char;
    uint16_t m_ForeColorIdx;
    uint16_t m_BackColorIdx;
    std::bitset<16> m_Mode;
    bool m_IsWideChar;
};

TermCellPtr CreateDefaultTermCell(TermLine * line)
{
    return TermCellPtr { new DefaultTermCell(line) };
}
