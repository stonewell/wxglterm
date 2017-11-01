#include "default_term_selection_decl.h"
#include "default_term_line.h"
#include "default_term_cell.h"

#include "internal_term_buffer.h"
#include "default_term_buffer_decl.h"

#include <vector>
#include <iostream>
#include <cassert>

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
    , m_VisRowHeaderBegin {0}
    , m_VisRowScrollRegionBegin {0}
    , m_VisRowFooterBegin {0}
{
}

void __InternalTermBuffer::Resize(uint32_t row, uint32_t col) {
    printf("++++Resize buffer: rows=%u, %u, cols=%u,%u\n",
           m_Rows, m_CurRow,
           m_Cols, m_CurCol);

    if (m_Rows == row && m_Cols == col)
        return;

    ClearHistoryLinesData();

    m_Rows = row;
    m_Cols = col;

    m_ScrollRegionBegin = 0;
    m_ScrollRegionEnd = 0;

    if (m_CurRow >= m_Rows)
        SetRow(m_Rows ? m_Rows - 1 : 0);

    if (m_CurCol >= m_Cols)
        SetCol(m_Cols ? m_Cols - 1 : 0);

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

    m_VisRowHeaderBegin = 0;
    m_VisRowFooterBegin = row;
    m_VisRowScrollRegionBegin = 0;
}

bool __InternalTermBuffer::HasScrollRegion()
{
    return m_ScrollRegionBegin < m_ScrollRegionEnd;
}

uint32_t __InternalTermBuffer::RowToLineIndex(uint32_t row)
{
    if (HasScrollRegion())
    {
        if (row < m_ScrollRegionBegin)
            return m_VisRowHeaderBegin + row;
        else if (row >= m_ScrollRegionBegin && row <= m_ScrollRegionEnd)
            return m_VisRowScrollRegionBegin + row - m_ScrollRegionBegin;
        else
            return m_VisRowFooterBegin + row - m_ScrollRegionEnd - 1;
    }
    else
    {
        return m_VisRowHeaderBegin + row;
    }
}

bool  __InternalTermBuffer::__NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(uint32_t & begin,
                                                                                    uint32_t count,
                                                                                    uint32_t & end)
{
    end = begin + count;

    if (HasScrollRegion())
    {
        if (begin < m_ScrollRegionBegin)
            begin = m_ScrollRegionBegin;

        if (end > m_ScrollRegionEnd)
        {
            //Reset line directly
            for (uint32_t i = begin;i <= m_ScrollRegionEnd; i++)
            {
                auto line = GetLine(i);

                if (line)
                {
                    line->Resize(0);
                    line->Resize(GetCols());
                }
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
                auto line = GetLine(i);

                if (line)
                {
                    line->Resize(0);
                    line->Resize(GetCols());
                }
            }
            return true;
        }

        end = m_Rows;
    }

    return false;
}

void __InternalTermBuffer::SetCurCellData(uint32_t ch,
                                          bool wide_char,
                                          bool insert,
                                          TermCellPtr cell_template) {
    int new_cell_count = wide_char ? 2 : 1;

    if (!insert)
    {
        if (m_CurCol + new_cell_count > m_Cols) {
            MoveCurRow(1, true, false);
            SetCol(0);
        }

        TermCellPtr cell = GetCurCell();
        cell->Reset(cell_template);
        cell->SetChar((wchar_t)ch);
        cell->SetWideChar(wide_char);
        SetCol(m_CurCol + 1);

        if (wide_char) {
            cell = GetCurCell();
            cell->Reset(cell_template);
            cell->SetChar((wchar_t)0);
            SetCol(m_CurCol + 1);
        }
    } else {
        uint32_t saved_row = m_CurRow;
        uint32_t saved_col = m_CurCol;

        TermLinePtr line = GetLine(m_CurRow);

        TermCellPtr extra_cell = line->InsertCell(m_CurCol);

        TermCellPtr cell = line->GetCell(m_CurCol);
        cell->Reset(cell_template);
        cell->SetChar((wchar_t)ch);
        cell->SetWideChar(wide_char);
        SetCol(m_CurCol + 1);

        TermCellPtr extra_cell_2{};

        if (wide_char) {
            extra_cell_2 = line->InsertCell(m_CurCol);

            cell = line->GetCell(m_CurCol);
            cell->Reset(cell_template);
            cell->SetChar((wchar_t)0);
            SetCol(m_CurCol + 1);
        }

        if (!IsDefaultCell(extra_cell) || !IsDefaultCell(extra_cell_2)) {
            MoveCurRow(1, true, false);
            SetCol(0);

            if (m_CurRow > saved_row)
            {
                if (!IsDefaultCell(extra_cell)) {
                    SetCurCellData((uint32_t)extra_cell->GetChar(),
                                   extra_cell->IsWideChar(),
                                   insert,
                                   extra_cell);
                }

                if (!IsDefaultCell(extra_cell_2)) {
                    SetCurCellData((uint32_t)extra_cell_2->GetChar(),
                                   extra_cell_2->IsWideChar(),
                                   insert,
                                   extra_cell_2);
                }
            }
        }

        SetRow(saved_row);
        SetCol(saved_col);
    }

    if (m_CurCol >= m_Cols)
        SetCol(m_Cols - 1);
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
        auto index = RowToLineIndex(row);
        auto line =  m_Lines[index];
        if (!line) {
            line = CreateDefaultTermLine(m_TermBuffer);
            line->Resize(GetCols());
            m_Lines[index] = line;
        }

        return line;
    }

    printf("invalid row:%u, rows:%u\n", row, GetRows());
    return TermLinePtr{};
}

void __InternalTermBuffer::DeleteLines(uint32_t begin, uint32_t count) {
    uint32_t end = m_Rows;

    //delete/insert lines will clear all saved history data
    ClearHistoryLinesData();

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
        tmpVector.push_back(TermLinePtr{});
    }

    TermLineVector::iterator b_it = m_Lines.begin() + RowToLineIndex(begin),
            e_it = m_Lines.begin() + RowToLineIndex(end);

    m_Lines.insert(e_it, tmpVector.begin(), tmpVector.end());

    //recalculate iterator
    b_it = m_Lines.begin() + RowToLineIndex(begin);
    e_it = m_Lines.begin() + RowToLineIndex(begin + count);
    m_Lines.erase(b_it, e_it);
}

void __InternalTermBuffer::InsertLines(uint32_t begin, uint32_t count) {
    uint32_t end = m_Rows;

    //delete/insert lines will clear all saved history data
    ClearHistoryLinesData();

    if (__NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(begin,
                                                                count,
                                                                end))
    {
        return;
    }

    if (end <= begin)
        return;

    TermLineVector::iterator b_it = m_Lines.begin() + RowToLineIndex(end - count),
            e_it = m_Lines.begin() + RowToLineIndex(end);

    m_Lines.erase(b_it, e_it);

    TermLineVector tmpVector;
    for (uint32_t i = 0; i < count; i++)
    {
        tmpVector.push_back(TermLinePtr{});
    }

    b_it = m_Lines.begin() + RowToLineIndex(begin);
    m_Lines.insert(b_it, tmpVector.begin(), tmpVector.end());
}

void __InternalTermBuffer::ScrollBuffer(int32_t scroll_offset) {
    if (scroll_offset < 0) {
        if (HasScrollRegion()) {
            uint32_t begin = m_VisRowScrollRegionBegin - scroll_offset;
            uint32_t end = begin + m_ScrollRegionEnd - m_ScrollRegionBegin + 1;

            while (end > m_VisRowFooterBegin) {
                m_Lines.insert(m_Lines.begin() + m_VisRowFooterBegin,
                               TermLinePtr{});
                m_VisRowFooterBegin++;
            }

            m_VisRowScrollRegionBegin = begin;
        } else {
            uint32_t begin = m_VisRowHeaderBegin - scroll_offset;
            uint32_t end = begin + m_Rows;

            if (m_Lines.size() < end) {
                m_Lines.resize(end);
            }

            m_VisRowHeaderBegin = begin;
            m_VisRowFooterBegin = end;
        }
    } else if (scroll_offset > 0) {
        if (HasScrollRegion()) {
            if (m_VisRowHeaderBegin + m_ScrollRegionBegin + scroll_offset > m_VisRowScrollRegionBegin) {
                for(uint32_t i = m_VisRowScrollRegionBegin;
                    i < m_VisRowHeaderBegin + m_ScrollRegionBegin + scroll_offset;
                    i++) {
                    m_Lines.insert(m_Lines.begin() + m_VisRowHeaderBegin + m_ScrollRegionBegin,
                                   TermLinePtr{});
                    m_VisRowFooterBegin ++;
                }

                m_VisRowScrollRegionBegin = m_VisRowHeaderBegin + m_ScrollRegionBegin + scroll_offset;
            }

            m_VisRowScrollRegionBegin -= scroll_offset;
        } else {
            if (m_VisRowHeaderBegin < (uint32_t)scroll_offset) {
                for(uint32_t i = m_VisRowHeaderBegin;i < (uint32_t)scroll_offset;i++) {
                    m_Lines.insert(m_Lines.begin(), TermLinePtr{});
                }

                m_VisRowHeaderBegin = scroll_offset;
            }

            m_VisRowHeaderBegin -= scroll_offset;
            m_VisRowFooterBegin = m_VisRowHeaderBegin + m_Rows;
        }
    }
}

bool __InternalTermBuffer::MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer) {
    uint32_t begin = 0, end = GetRows() - 1;

    bool scrolled = false;
    if (HasScrollRegion()) {
        begin = m_ScrollRegionBegin;
        end = m_ScrollRegionEnd;
    }

    if (move_down) {
        if (m_CurRow + offset <= end) {
            SetRow(m_CurRow + offset);
        } else {
            SetRow(end);

            //scroll
            if (scroll_buffer)
            {
                ScrollBuffer(-1 * (m_CurRow + offset - end));
                scrolled = true;
            }
        }
    } else {
        if (m_CurRow >= offset && (m_CurRow - offset) >= begin) {
            SetRow(m_CurRow - offset);
        } else {
            SetRow(begin);

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

void __InternalTermBuffer::SetRow(uint32_t row) {
    m_CurRow = row;
}

void __InternalTermBuffer::SetCol(uint32_t col) {
    m_CurCol = col;
}

void __InternalTermBuffer::SetScrollRegionBegin(uint32_t begin) {
    ClearHistoryLinesData();
    m_ScrollRegionBegin = begin;

    m_VisRowScrollRegionBegin = HasScrollRegion() ? m_VisRowHeaderBegin + begin : 0;
    m_VisRowFooterBegin = HasScrollRegion() ? m_ScrollRegionEnd + 1 : m_Rows;
}

void __InternalTermBuffer::SetScrollRegionEnd(uint32_t end) {
    ClearHistoryLinesData();
    m_ScrollRegionEnd = end;

    m_VisRowScrollRegionBegin = HasScrollRegion() ? m_VisRowHeaderBegin + m_ScrollRegionBegin : 0;
    m_VisRowFooterBegin = HasScrollRegion() ? m_ScrollRegionEnd + 1 : m_Rows;
}

void __InternalTermBuffer::ClearHistoryLinesData() {
    printf("++++ClearHistoryData: rows=%u, %zu\n",
           m_Rows, m_Lines.size());

    auto it = m_Lines.begin();

    if (HasScrollRegion()) {
        it = m_Lines.begin() + m_VisRowFooterBegin + m_Rows - m_ScrollRegionEnd - 1;
        if (it != m_Lines.end())
            m_Lines.erase(it, m_Lines.end());

        it = m_Lines.begin() + m_VisRowScrollRegionBegin + m_ScrollRegionEnd - m_ScrollRegionBegin + 1;

        if (it != m_Lines.end()
            && (m_VisRowScrollRegionBegin + m_ScrollRegionEnd - m_ScrollRegionBegin + 1 < m_VisRowFooterBegin))
            m_Lines.erase(it,
                          m_Lines.begin() + m_VisRowFooterBegin);

        it = m_Lines.begin() + m_VisRowHeaderBegin + m_ScrollRegionBegin;

        if (it != m_Lines.end()
            && (m_VisRowHeaderBegin + m_ScrollRegionBegin < m_VisRowScrollRegionBegin))
            m_Lines.erase(it,
                          m_Lines.begin() + m_VisRowScrollRegionBegin);
    } else {
        it = m_Lines.begin() + m_VisRowHeaderBegin + m_Rows;

        if (it != m_Lines.end())
            m_Lines.erase(it, m_Lines.end());
    }

    it = m_Lines.begin();

    if (it != m_Lines.end()
        && (m_VisRowHeaderBegin > 0))
        m_Lines.erase(it, m_Lines.begin() + m_VisRowHeaderBegin);

    if (m_Lines.size() > 0)
        assert(m_Lines.size() >= m_Rows);

    m_VisRowHeaderBegin = 0;
    m_VisRowFooterBegin = HasScrollRegion() ? m_ScrollRegionEnd + 1 : m_Rows;

    m_VisRowScrollRegionBegin = HasScrollRegion() ? m_ScrollRegionBegin : 0;
}
