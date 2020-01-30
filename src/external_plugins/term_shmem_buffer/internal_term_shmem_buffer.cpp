#include "default_term_selection_decl.h"
#include "default_term_line.h"

#include "internal_term_shmem_buffer.h"
#include "term_shmem_buffer_decl.h"

#include <vector>
#include <iostream>
#include <cassert>
#include <functional>

InternalTermShmemBuffer::InternalTermShmemBuffer(TermShmemBuffer* term_buffer)
    : m_TermBuffer {term_buffer}
    , m_Rows {0}
    , m_Cols {0}
    , m_CurRow {0}
    , m_CurCol {0}
    , m_ScrollRegionBegin {0}
    , m_ScrollRegionEnd {0}
    , m_Lines{}
    , m_Selection{new DefaultTermSelection}
    , m_VisRowHeaderBegin {0}
    , m_VisRowScrollRegionBegin {0}
    , m_VisRowFooterBegin {0}
    , m_Mode {0}
    , m_Storage {CreateTermShmemStorage(1)}
    , m_TermCellPool {CreateRawTermCell}
{
}

void InternalTermShmemBuffer::Resize(uint32_t row, uint32_t col) {
    std::function<void()> reset_lines { [this]() {
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

        }
    };

    if (m_Rows == row && m_Cols == col) {
        reset_lines();
        return;
    }

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

    reset_lines();

    ClearSelection();

    m_VisRowHeaderBegin = 0;
    m_VisRowFooterBegin = row;
    m_VisRowScrollRegionBegin = 0;
}

bool InternalTermShmemBuffer::HasScrollRegion()
{
    return m_ScrollRegionBegin < m_ScrollRegionEnd;
}

uint32_t InternalTermShmemBuffer::RowToLineIndex(uint32_t row)
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

bool  InternalTermShmemBuffer::__NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(uint32_t & begin,
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

void InternalTermShmemBuffer::SetCurCellData(uint32_t ch,
                                          bool wide_char,
                                          bool insert,
                                          TermCellPtr cell_template) {
    int new_cell_count = wide_char ? 2 : 1;

    if (m_TermBuffer->m_Debug)
        std::cout << "set:[" << (char)ch << "], c:" << m_CurCol << ", r:" << m_CurRow << std::endl;

    if (!insert)
    {
        if (m_CurCol + new_cell_count > m_Cols) {
            MoveCurRow(1, true, false, cell_template);
            SetCol(0);
        }

        TermCellPtr cell = GetCurCell();

        if (!cell) return;

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
            MoveCurRow(1, true, false, cell_template);
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

    if (m_CurCol > m_Cols)
        SetCol(m_Cols - 1);
}

bool InternalTermShmemBuffer::IsDefaultCell(TermCellPtr cell) {
    if (!cell)
        return true;

    return (cell->GetChar() == ' ' &&
            cell->GetForeColorIndex() == m_TermBuffer->m_DefaultForeColorIndex &&
            cell->GetBackColorIndex() == m_TermBuffer->m_DefaultBackColorIndex &&
            cell->GetMode() == m_TermBuffer->m_DefaultMode);
}

TermLinePtr InternalTermShmemBuffer::GetLine(uint32_t row) {
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

TermCellPtr InternalTermShmemBuffer::GetCell(uint32_t row, uint32_t col) {
    auto line = GetLine(row);

    if (line)
        return line->GetCell(col);

    return TermCellPtr{};
}

void InternalTermShmemBuffer::DeleteLines(uint32_t begin, uint32_t count, TermCellPtr cell_template) {
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
        TermLinePtr new_line = CreateDefaultTermLine(m_TermBuffer);
        new_line->Resize(GetCols(), cell_template);
        tmpVector.push_back(new_line);
    }

    TermLineVector::iterator b_it = m_Lines.begin() + RowToLineIndex(begin),
            e_it = m_Lines.begin() + RowToLineIndex(end);

    m_Lines.insert(e_it, tmpVector.begin(), tmpVector.end());

    //recalculate iterator
    b_it = m_Lines.begin() + RowToLineIndex(begin);
    e_it = m_Lines.begin() + RowToLineIndex(begin + count);
    m_Lines.erase(b_it, e_it);
}

void InternalTermShmemBuffer::InsertLines(uint32_t begin, uint32_t count, TermCellPtr cell_template) {
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
        TermLinePtr new_line = CreateDefaultTermLine(m_TermBuffer);
        new_line->Resize(GetCols(), cell_template);
        tmpVector.push_back(new_line);
    }

    b_it = m_Lines.begin() + RowToLineIndex(begin);
    m_Lines.insert(b_it, tmpVector.begin(), tmpVector.end());
}

void InternalTermShmemBuffer::ScrollBuffer(int32_t scroll_offset, TermCellPtr cell_template) {
    if (scroll_offset < 0) {
        if (HasScrollRegion()) {
            uint32_t begin = m_VisRowScrollRegionBegin - scroll_offset;
            uint32_t end = begin + m_ScrollRegionEnd - m_ScrollRegionBegin + 1;

            while (end > m_VisRowFooterBegin) {
                TermLinePtr new_line = CreateDefaultTermLine(m_TermBuffer);
                new_line->Resize(GetCols(), cell_template);
                m_Lines.insert(m_Lines.begin() + m_VisRowFooterBegin,
                               new_line);
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
                    TermLinePtr new_line = CreateDefaultTermLine(m_TermBuffer);
                    new_line->Resize(GetCols(), cell_template);
                    m_Lines.insert(m_Lines.begin() + m_VisRowHeaderBegin + m_ScrollRegionBegin,
                                   new_line);
                    m_VisRowFooterBegin ++;
                }

                m_VisRowScrollRegionBegin = m_VisRowHeaderBegin + m_ScrollRegionBegin + scroll_offset;
            }

            m_VisRowScrollRegionBegin -= scroll_offset;
        } else {
            if (m_VisRowHeaderBegin < (uint32_t)scroll_offset) {
                for(uint32_t i = m_VisRowHeaderBegin;i < (uint32_t)scroll_offset;i++) {
                    TermLinePtr new_line = CreateDefaultTermLine(m_TermBuffer);
                    new_line->Resize(GetCols(), cell_template);
                    m_Lines.insert(m_Lines.begin(), new_line);
                }

                m_VisRowHeaderBegin = scroll_offset;
            }

            m_VisRowHeaderBegin -= scroll_offset;
            m_VisRowFooterBegin = m_VisRowHeaderBegin + m_Rows;
        }
    }
}

bool InternalTermShmemBuffer::MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer, TermCellPtr cell_template) {
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
                ScrollBuffer(-1 * (m_CurRow + offset - end), cell_template);
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
                ScrollBuffer(begin + offset - m_CurRow, cell_template);
                scrolled = true;
            }
        }
    }

    return scrolled;
}

void InternalTermShmemBuffer::SetRow(uint32_t row) {
    m_CurRow = row;
}

void InternalTermShmemBuffer::SetCol(uint32_t col) {
    m_CurCol = col;
}

void InternalTermShmemBuffer::SetScrollRegionBegin(uint32_t begin) {
    ClearHistoryLinesData();
    m_ScrollRegionBegin = begin;

    m_VisRowScrollRegionBegin = HasScrollRegion() ? m_VisRowHeaderBegin + begin : 0;
    m_VisRowFooterBegin = HasScrollRegion() ? m_ScrollRegionEnd + 1 : m_Rows;
}

void InternalTermShmemBuffer::SetScrollRegionEnd(uint32_t end) {
    ClearHistoryLinesData();
    m_ScrollRegionEnd = end;

    m_VisRowScrollRegionBegin = HasScrollRegion() ? m_VisRowHeaderBegin + m_ScrollRegionBegin : 0;
    m_VisRowFooterBegin = HasScrollRegion() ? m_ScrollRegionEnd + 1 : m_Rows;
}

void InternalTermShmemBuffer::ClearHistoryLinesData() {
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

uint32_t InternalTermShmemBuffer::GetMode() const {
    return (uint32_t)m_Mode.to_ulong();
}

void InternalTermShmemBuffer::SetMode(uint32_t m) {
    m_Mode = std::bitset<16>(m);
}

void InternalTermShmemBuffer::AddMode(uint32_t m) {
    m_Mode.set(m);
}
void InternalTermShmemBuffer::RemoveMode(uint32_t m) {
    m_Mode.reset(m);
}
