#include "plugin_base.h"

#include "default_term_cell.h"
#include <vector>

class DefaultTermCell : public virtual PluginBase, public virtual TermCell {
public:
    DefaultTermCell(TermLine * term_line) :
        PluginBase("default_term_cell", "default terminal cell plugin", 0)
        , m_TermLine(term_line)
    {
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
        return m_Mode;
    }

    void SetMode(uint16_t m) override {
        m_Mode = m;
    }

private:
    TermLine * m_TermLine;

    wchar_t m_Char;
    uint16_t m_ForeColorIdx;
    uint16_t m_BackColorIdx;
    uint16_t m_Mode;
};

TermCellPtr CreateDefaultTermCell(TermLine * line)
{
    return TermCellPtr { new DefaultTermCell(line) };
}
