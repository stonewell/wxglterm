#include "plugin_base.h"

#include "default_term_buffer.h"
#include "default_term_line.h"
#include "default_term_cell.h"
#include "term_selection.h"

#include <vector>

class DefaultTermSelection : public virtual PluginBase, public virtual TermSelection {
public:
    DefaultTermSelection() :
        PluginBase("default_term_buffer_cpp", "default terminal buffer plugin", 1)
        , m_RowBegin{0}
        , m_ColBegin(0)
        , m_RowEnd{0}
        , m_ColEnd(0)
    {
    }

    virtual ~DefaultTermSelection() = default;

    uint32_t GetRowBegin() const override {
        return m_RowBegin;
    }

    void SetRowBegin(uint32_t rowBegin) override {
        m_RowBegin = rowBegin;
    }
    uint32_t GetColBegin() const override {
        return m_ColBegin;
    }

    void SetColBegin(uint32_t colBegin) override {
        m_ColBegin = colBegin;
    }

    uint32_t GetRowEnd() const override {
        return m_RowEnd;
    }
    void SetRowEnd(uint32_t rowEnd) override {
        m_RowEnd = rowEnd;
    }
    uint32_t GetColEnd() const override {
        return m_ColEnd;
    }
    void SetColEnd(uint32_t colEnd) override {
        m_ColEnd = colEnd;
    }
private:
    uint32_t m_RowBegin, m_ColBegin;
    uint32_t m_RowEnd, m_ColEnd;
};

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
        , m_Selection{new DefaultTermSelection}
    {
    }

    virtual ~DefaultTermBuffer() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new DefaultTermBuffer()};
    }

    void Resize(uint32_t row, uint32_t col) override {
        m_Rows = row;
        m_Cols = col;

        m_ScrollRegionBegin = 0;
        m_ScrollRegionEnd = row - 1;

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
        if (begin + count > m_ScrollRegionEnd)
        {
            //Reset line directly
            for (uint32_t i = begin;i <= m_ScrollRegionEnd; i++)
            {
                m_Lines[i]->Resize(0);
                m_Lines[i]->Resize(GetCols());
            }
            return;
        }

        uint32_t end = begin + count;

        TermLineVector::iterator b_it = m_Lines.begin() + begin,
                e_it = m_Lines.begin() + end;

        //Insert First, then delete
        for (uint32_t i = begin; i < end; i++)
        {
            auto term_line = CreateDefaultTermLine(this);
            term_line->Resize(GetCols());
            m_Lines.insert(e_it, term_line);
        }

        m_Lines.erase(b_it, e_it);
    }

    void InsertLines(uint32_t begin, uint32_t count) override {
        if (begin + count > m_ScrollRegionEnd)
        {
            //Reset line directly
            for (uint32_t i = begin;i <= m_ScrollRegionEnd; i++)
            {
                m_Lines[i]->Resize(0);
                m_Lines[i]->Resize(GetCols());
            }
            return;
        }

        TermLineVector::iterator b_it = m_Lines.begin() + m_ScrollRegionEnd - count + 1,
                e_it = m_Lines.begin() + m_ScrollRegionEnd + 1;

        m_Lines.erase(b_it, e_it);

        b_it = m_Lines.begin() + begin;

        for (uint32_t i = 0; i < count; i++)
        {
            auto term_line = CreateDefaultTermLine(this);
            term_line->Resize(GetCols());
            m_Lines.insert(b_it, term_line);
        }
    }

    void ScrollBuffer(int32_t scroll_offset) override {
        TermLineVector::iterator b_it = m_Lines.begin(),
                e_it = m_Lines.end();

        if (m_ScrollRegionBegin < m_ScrollRegionEnd) {
            b_it = b_it + m_ScrollRegionBegin;
            e_it = b_it + m_ScrollRegionEnd + 1;
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

    void MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer) override {
        uint32_t begin = 0, end = GetRows() - 1;

        if (m_ScrollRegionBegin < m_ScrollRegionEnd) {
            begin = m_ScrollRegionBegin;
            end = m_ScrollRegionEnd;
        }

        if (move_down) {
            if (m_CurRow + offset <= end) {
                m_CurRow += offset;
            } else {
                m_CurRow = end;

                //scroll
                if (scroll_buffer)
                    ScrollBuffer(m_CurRow + offset - end);
            }
        } else {
            if (m_CurRow >= offset && (m_CurRow - offset) >= begin) {
                m_CurRow -= offset;
            } else {
                m_CurRow = begin;

                //scroll
                if (!scroll_buffer)
                    ScrollBuffer(-1 * (begin + offset - m_CurRow));
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
        m_Selection->SetRowBegin(selection->GetRowBegin());
        m_Selection->SetRowEnd(selection->GetRowEnd());
        m_Selection->SetColBegin(selection->GetColBegin());
        m_Selection->SetColEnd(selection->GetColEnd());
    }

    TermSelectionPtr GetSelection() override {
        return m_Selection;
    }

    void ClearSelection() override {
        m_Selection->SetRowBegin(0);
        m_Selection->SetRowEnd(0);
        m_Selection->SetColBegin(0);
        m_Selection->SetColEnd(0);
    }

    void SetCurCellData(uint32_t ch, bool wide_char, bool insert, TermCellPtr cell_template) override {
        int new_cell_count = wide_char ? 1 : 2;

        if (!insert)
        {
            if (m_CurCol + new_cell_count > m_Cols) {
                MoveCurRow(1, true, false);
                m_CurCol = 0;
            }

            TermCellPtr cell = GetCurCell();
            cell->Reset(cell_template);
            cell->SetChar((wchar_t)ch);
            cell->SetWideChar(wide_char);
            m_CurCol++;

            if (wide_char) {
                cell = GetCurCell();
                cell->Reset(cell_template);
                cell->SetChar((wchar_t)0);
                m_CurCol++;
            }
        } else {
            TermLinePtr line = GetLine(m_CurRow);

            TermCellPtr extra_cell = line->InsertCell(m_CurCol);

            TermCellPtr cell = line->GetCell(m_CurCol);
            cell->Reset(cell_template);
            cell->SetChar((wchar_t)ch);
            cell->SetWideChar(wide_char);
            m_CurCol++;

            TermCellPtr extra_cell_2{};

            if (wide_char) {
                extra_cell_2 = line->InsertCell(m_CurCol);

                cell = line->GetCell(m_CurCol);
                cell->Reset(cell_template);
                cell->SetChar((wchar_t)0);
                m_CurCol++;
            }

            uint32_t saved_row = m_CurRow;
            uint32_t saved_col = m_CurCol;

            if (extra_cell || extra_cell_2) {
                MoveCurRow(1, true, false);
                m_CurCol = 0;

                if (extra_cell) {
                    SetCurCellData((uint32_t)extra_cell->GetChar(),
                                   extra_cell->IsWideChar(),
                                   insert,
                                   extra_cell);
                }

                if (extra_cell_2) {
                    SetCurCellData((uint32_t)extra_cell_2->GetChar(),
                                   extra_cell_2->IsWideChar(),
                                   insert,
                                   extra_cell_2);
                }
            }

            m_CurRow = saved_row;
            m_CurCol = saved_col;
        }
    }
private:
    uint32_t m_Rows;
    uint32_t m_Cols;

    uint32_t m_CurRow;
    uint32_t m_CurCol;

    uint32_t m_ScrollRegionBegin;
    uint32_t m_ScrollRegionEnd;

    TermLineVector m_Lines;

    wchar_t m_DefaultChar;
    uint16_t m_DefaultForeColorIndex;
    uint16_t m_DefaultBackColorIndex;
    uint16_t m_DefaultMode;

    TermSelectionPtr m_Selection;
};

TermBufferPtr CreateDefaultTermBuffer() {
    return TermBufferPtr { new DefaultTermBuffer() };
}
