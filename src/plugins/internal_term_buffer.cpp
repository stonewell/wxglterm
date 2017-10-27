#include "default_term_selection_decl.h"
#include "default_term_line.h"
#include "default_term_cell.h"

#include "internal_term_buffer.h"
#include "default_term_buffer_decl.h"

#include <vector>
#include <iostream>

__InternalTermBuffer::__InternalTermBuffer(DefaultTermBuffer* term_buffer) :
    m_TermBuffer(term_buffer)
    , m_Rows(0)
    , m_Cols(0)
    , m_CurRow(0)
    , m_CurCol(0)
    , m_ScrollRegionBegin(0)
    , m_ScrollRegionEnd(0)
    , m_Lines()
    , m_Selection{new DefaultTermSelection}
{
}

void __InternalTermBuffer::Resize(uint32_t row, uint32_t col) {
    printf("++++Resize buffer: rows=%u, %u, cols=%u,%u\n",
           m_Rows, m_CurRow,
           m_Cols, m_CurCol);

    if (m_Rows == row && m_Cols == col)
        return;

    m_Rows = row;
    m_Cols = col;

    m_ScrollRegionBegin = 0;
    m_ScrollRegionEnd = row ? row - 1 : 0;

    if (m_CurRow >= m_Rows)
        m_CurRow = m_Rows ? m_Rows - 1 : 0;

    if (m_CurCol >= m_Cols)
        m_CurCol = m_Cols ? m_Cols - 1 : 0;

    m_Lines.resize(m_Rows);

    printf("----Resize buffer: rows=%u, %u, cols=%u,%u\n",
           m_Rows, m_CurRow,
           m_Cols, m_CurCol);

    for (TermLineVector::iterator it = m_Lines.begin(),
                 it_end = m_Lines.end();
         it != it_end;
         it++)
    {
        if (*it)
        {
            (*it)->Resize(m_Cols);
        }
    }

    ClearSelection();
}


bool  __InternalTermBuffer::__NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(uint32_t & begin,
                                                                                    uint32_t count,
                                                                                    uint32_t & end)
{
    end = begin + count;

    if (m_ScrollRegionBegin < m_ScrollRegionEnd)
    {
        if (begin < m_ScrollRegionBegin)
            begin = m_ScrollRegionBegin;

        if (end > m_ScrollRegionEnd)
        {
            //Reset line directly
            for (uint32_t i = begin;i <= m_ScrollRegionEnd; i++)
            {
                m_Lines[i]->Resize(0);
                m_Lines[i]->Resize(GetCols());
            }
            return true;
        }

        end = m_ScrollRegionEnd + 1;
    }
    else
    {
        if (end >= m_Rows)
        {
            //Reset line directly
            for (uint32_t i = begin;i < m_Rows; i++)
            {
                m_Lines[i]->Resize(0);
                m_Lines[i]->Resize(GetCols());
            }
            return true;
        }

        end = m_Rows;
    }

    return false;
}

TermLineVector::iterator  __InternalTermBuffer::__GetScrollBeginIt() {
    TermLineVector::iterator _it = m_Lines.begin();

    if (m_ScrollRegionBegin < m_ScrollRegionEnd) {
        _it = _it + m_ScrollRegionBegin;
    }

    return _it;
};

TermLineVector::iterator  __InternalTermBuffer::__GetScrollEndIt() {
    TermLineVector::iterator _it = m_Lines.end();

    if (m_ScrollRegionBegin < m_ScrollRegionEnd) {
        _it = m_Lines.begin() + m_ScrollRegionEnd + 1;
    }

    return _it;
};

void __InternalTermBuffer::SetCurCellData(uint32_t ch, bool wide_char, bool insert, TermCellPtr cell_template) {
    int new_cell_count = wide_char ? 2 : 1;

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

        if (!IsDefaultCell(extra_cell) || !IsDefaultCell(extra_cell_2)) {
            MoveCurRow(1, true, false);
            m_CurCol = 0;

            if (m_CurRow >= saved_row)
            {
                if (extra_cell) {
                    printf("extra_cell:%d\n", (uint32_t)extra_cell->GetChar());
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
        }

        m_CurRow = saved_row;
        m_CurCol = saved_col;
    }

    if (m_CurCol >= m_Cols)
        m_CurCol = m_Cols - 1;
}

bool __InternalTermBuffer::IsDefaultCell(TermCellPtr cell) {
    if (!cell)
        return true;

    return (cell->GetChar() == ' ' &&
            cell->GetForeColorIndex() == m_TermBuffer->m_DefaultForeColorIndex &&
            cell->GetBackColorIndex() == m_TermBuffer->m_DefaultBackColorIndex &&
            cell->GetMode() == m_TermBuffer->m_DefaultMode);
}

TermLinePtr __InternalTermBuffer::GetLine(uint32_t row) {
    if (row < GetRows()) {
        auto line =  m_Lines[row];
        if (!line) {
            line = CreateDefaultTermLine(m_TermBuffer);
            line->Resize(GetCols());
            m_Lines[row] = line;
        }

        return line;
    }

    printf("invalid row:%u, rows:%u\n", row, GetRows());
    return TermLinePtr{};
}

void __InternalTermBuffer::DeleteLines(uint32_t begin, uint32_t count) {
    uint32_t end = m_Rows;

    if (__NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(begin,
                                                                count,
                                                                end))
    {
        return;
    }

    if (end <= begin)
        return;

    TermLineVector tmpVector;
    //Insert First, then delete
    for (uint32_t i = begin; i < begin + count; i++)
    {
        auto term_line = CreateDefaultTermLine(m_TermBuffer);
        term_line->Resize(GetCols());
        tmpVector.push_back(term_line);
    }

    TermLineVector::iterator b_it = m_Lines.begin() + begin,
            e_it = m_Lines.begin() + end;

    m_Lines.insert(e_it, tmpVector.begin(), tmpVector.end());

    //recalculate iterator
    b_it = m_Lines.begin() + begin;
    e_it = b_it + count;
    m_Lines.erase(b_it, e_it);
}

void __InternalTermBuffer::InsertLines(uint32_t begin, uint32_t count) {
    uint32_t end = m_Rows;

    if (__NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(begin,
                                                                count,
                                                                end))
    {
        return;
    }

    if (end <= begin)
        return;

    TermLineVector::iterator b_it = m_Lines.begin() + end - count,
            e_it = m_Lines.begin() + end;

    m_Lines.erase(b_it, e_it);

    TermLineVector tmpVector;
    for (uint32_t i = 0; i < count; i++)
    {
        auto term_line = CreateDefaultTermLine(m_TermBuffer);
        term_line->Resize(GetCols());
        tmpVector.push_back(term_line);
    }

    b_it = m_Lines.begin() + begin;
    m_Lines.insert(b_it, tmpVector.begin(), tmpVector.end());
}

void __InternalTermBuffer::ScrollBuffer(int32_t scroll_offset) {
    if (scroll_offset < 0) {
        TermLineVector tmpVector;
        for(int i=0;i < -scroll_offset;i++) {
            auto term_line = CreateDefaultTermLine(m_TermBuffer);
            term_line->Resize(GetCols());
            tmpVector.push_back(term_line);
        }

        m_Lines.insert(__GetScrollEndIt(), tmpVector.begin(), tmpVector.end());

        TermLineVector::iterator b_it = __GetScrollBeginIt();

        m_Lines.erase(b_it, b_it - scroll_offset);
    } else if (scroll_offset > 0) {
        TermLineVector::iterator e_it = __GetScrollEndIt();

        m_Lines.erase(e_it - scroll_offset, e_it);

        TermLineVector tmpVector;
        for(int i=0;i < scroll_offset;i++) {
            auto term_line = CreateDefaultTermLine(m_TermBuffer);
            term_line->Resize(GetCols());
            tmpVector.push_back(term_line);
        }

        //recalculate
        m_Lines.insert(__GetScrollBeginIt(), tmpVector.begin(), tmpVector.end());
    }
}

bool __InternalTermBuffer::MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer) {
    uint32_t begin = 0, end = GetRows() - 1;

    bool scrolled = false;
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
            {
                ScrollBuffer(-1 * (m_CurRow + offset - end));
                scrolled = true;
            }
        }
    } else {
        if (m_CurRow >= offset && (m_CurRow - offset) >= begin) {
            m_CurRow -= offset;
        } else {
            m_CurRow = begin;

            //scroll
            if (scroll_buffer)
            {
                ScrollBuffer(begin + offset - m_CurRow);
                scrolled = true;
            }
        }
    }

    return scrolled;
}
