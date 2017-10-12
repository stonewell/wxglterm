#include "plugin_base.h"

#include "default_term_buffer.h"
#include "default_term_line.h"
#include "default_term_cell.h"

#include <vector>

class DefaultTermBuffer : public virtual PluginBase, public virtual TermBuffer {
public:
    DefaultTermBuffer() :
        PluginBase("default_term_buffer", "default terminal buffer plugin", 0)
        , m_Rows(0)
        , m_Cols(0)
        , m_CurRow(0)
        , m_CurCol(0)
        , m_ScrollRegionBegin(0)
        , m_ScrollRegionEnd(0)
        , m_Lines()
        , m_DefaultChar(' ')
        , m_DefaultForeColorIndex(TermCell::DefaultColorIndex)
        , m_DefaultBackColorIndex(TermCell::DefaultColorIndex)
        , m_DefaultMode(0)
    {
    }

    virtual ~DefaultTermBuffer() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new DefaultTermBuffer()};
    }

    void Resize(uint32_t row, uint32_t col) override {
        m_Rows = row;
        m_Cols = col;

        m_Lines.resize(m_Rows);
    }

    uint32_t GetRows() const override {
        return m_Rows;
    }

    uint32_t GetCols() const override {
        return m_Cols;
    }

    uint32_t GetRow() const override {
        return m_CurRow;
    }

    uint32_t GetCol() const override {
        return m_CurCol;
    }
    void SetRow(uint32_t row) override {
        m_CurRow = row;
    }

    void SetCol(uint32_t col) override {
        m_CurCol = col;
    }

    TermLinePtr GetLine(uint32_t row) override {
        if (row < GetRows()) {
            auto line =  m_Lines[row];
            if (!line) {
                line = CreateDefaultTermLine(this);
                line->Resize(GetCols());
                m_Lines[row] = line;
            }

            return line;
        }

        return TermLinePtr{};
    }

    TermCellPtr GetCell(uint32_t row, uint32_t col) override {
        TermLinePtr line = GetLine(row);

        if (line)
            return line->GetCell(col);

        return TermCellPtr{};
    }

    TermLinePtr GetCurLine() override {
        return GetLine(GetRow());
    }

    TermCellPtr GetCurCell() override {
        return GetCell(GetRow(), GetCol());
    }

    uint32_t GetScrollRegionBegin() const override {
        return m_ScrollRegionBegin;
    }

    uint32_t GetScrollRegionEnd() const override {
        return m_ScrollRegionEnd;
    }

    void SetScrollRegionBegin(uint32_t begin) override {
        m_ScrollRegionBegin = begin;
    }

    void SetScrollRegionEnd(uint32_t end) override {
        m_ScrollRegionEnd = end;
    }

    void DeleteLines(uint32_t begin, uint32_t count) override {
        (void)begin;
        (void)count;
    }

    void InsertLines(uint32_t begin, uint32_t count) override {
        (void)begin;
        (void)count;
    }

    void ScrollBuffer(int32_t scroll_offset) override {
        std::vector<TermLinePtr>::iterator b_it = m_Lines.begin(),
                e_it = m_Lines.end();

        if (m_ScrollRegionBegin < m_ScrollRegionEnd) {
            b_it = b_it + m_ScrollRegionBegin;
            e_it = b_it + m_ScrollRegionEnd;
        }

        if (scroll_offset < 0) {
            m_Lines.erase(b_it, b_it - scroll_offset);

            for(int i=0;i < -scroll_offset;i++) {
                auto term_line = CreateDefaultTermLine(this);
                term_line->Resize(GetCols());
                m_Lines.insert(e_it, term_line);
            }
        } else if (scroll_offset > 0) {
            m_Lines.erase(e_it - scroll_offset, e_it);

            for(int i=0;i < scroll_offset;i++) {
                auto term_line = CreateDefaultTermLine(this);
                term_line->Resize(GetCols());
                m_Lines.insert(b_it, term_line);
            }
        }
    }

    void SetCellDefaults(wchar_t c,
                         uint16_t fore_color_idx,
                         uint16_t back_color_idx,
                         uint16_t mode) override {
        m_DefaultChar = c;
        m_DefaultForeColorIndex = fore_color_idx;
        m_DefaultBackColorIndex = back_color_idx;
        m_DefaultMode = mode;
    }

    TermCellPtr CreateCellWithDefaults() override {
        TermCellPtr cell = CreateDefaultTermCell(nullptr);

        cell->SetChar(m_DefaultChar);
        cell->SetForeColorIndex(m_DefaultForeColorIndex);
        cell->SetBackColorIndex(m_DefaultBackColorIndex);
        cell->SetMode(m_DefaultMode);

        return cell;
    }

    void SetSelection(TermSelectionPtr selection) override {
        (void)selection;
    }

    TermSelectionPtr GetSelection() override {
        return TermSelectionPtr{};
    }

    void ClearSelection() override {
        ;
    }
private:
    uint32_t m_Rows;
    uint32_t m_Cols;

    uint32_t m_CurRow;
    uint32_t m_CurCol;

    uint32_t m_ScrollRegionBegin;
    uint32_t m_ScrollRegionEnd;

    std::vector<TermLinePtr> m_Lines;

    wchar_t m_DefaultChar;
    uint16_t m_DefaultForeColorIndex;
    uint16_t m_DefaultBackColorIndex;
    uint16_t m_DefaultMode;
};

TermBufferPtr CreateDefaultTermBuffer() {
    return TermBufferPtr { new DefaultTermBuffer() };
}
