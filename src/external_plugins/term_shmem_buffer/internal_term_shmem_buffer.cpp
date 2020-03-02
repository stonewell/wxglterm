#include "default_term_selection_decl.h"

#include "internal_term_shmem_buffer.h"
#include "term_shmem_buffer_decl.h"

#include <vector>
#include <iostream>
#include <cassert>
#include <functional>
#include <cstring>

static
const uint8_t EMPTY_CELL_STORAGE[CELL_STORAGE_SIZE] = {0};

#define BYTES_COUNT(x, y) ((uint8_t*)(x) - (uint8_t*)(y))

InternalTermShmemBuffer::InternalTermShmemBuffer(TermShmemBuffer* term_buffer)
    : m_TermBuffer {term_buffer}
    , m_Rows {0}
    , m_Cols {0}
    , m_CurRow {0}
    , m_CurCol {0}
    , m_ScrollRegionBegin {0}
    , m_ScrollRegionEnd {0}
    , m_Selection{new DefaultTermSelection}
    , m_Mode {0}
    , m_Storage {CreateTermShmemStorage(1)}
    , m_LineSize {0}
    , m_BufferLineMapper {}
    , m_LinePtrs {}
{
}

void InternalTermShmemBuffer::Resize(uint32_t row, uint32_t col) {
    if (m_Rows == row && m_Cols == col) {
        return;
    }

    m_LinePtrs.resize(row);
    m_LineSize = LINE_STORAGE_SIZE + CELL_STORAGE_SIZE * col;
    size_t storage_size = m_LineSize * row;

    m_Storage = CreateTermShmemStorage(storage_size);

    auto buf = m_Storage->GetAddress();

    for(uint32_t i=0;i<row;i++) {
        LineStorage * line = (LineStorage *)(buf + m_LineSize * i);

        line->cols = col;
        line->last_render_index = row;
        line->modified = false;
    }

    m_Rows = row;
    m_Cols = col;

    m_ScrollRegionBegin = 0;
    m_ScrollRegionEnd = 0;

    if (m_CurRow >= m_Rows)
        SetRow(m_Rows ? m_Rows - 1 : 0);

    if (m_CurCol >= m_Cols)
        SetCol(m_Cols ? m_Cols - 1 : 0);

    ClearSelection();

    __CreateBufferLineMapper();
}

bool InternalTermShmemBuffer::HasScrollRegion()
{
    return m_ScrollRegionBegin < m_ScrollRegionEnd;
}

bool  InternalTermShmemBuffer::__NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(uint32_t & begin,
                                                                                       uint32_t count,
                                                                                       uint32_t & end)
{
    end = begin + count;

    bool reset_lines = false;

    if (HasScrollRegion())
    {
        if (begin < m_ScrollRegionBegin)
            begin = m_ScrollRegionBegin;

        if (end > m_ScrollRegionEnd)
        {
            reset_lines = true;
        }

        end = m_ScrollRegionEnd + 1;
    }
    else
    {
        if (end >= m_Rows)
        {
            reset_lines = true;
        }

        end = m_Rows;
    }

    if (reset_lines) {
        //Reset line directly
        for (uint32_t i = begin;i < end; i++)
        {
            LineStorage * begin_line = __GetLine(i);

            begin_line->cols = m_Cols;
            begin_line->last_render_index = m_Rows;
            begin_line->modified = false;
        }
    }

    return reset_lines;
}

void InternalTermShmemBuffer::SetCurCellData(uint32_t ch,
                                             bool wide_char,
                                             bool insert,
                                             const TermCellPtr & cell_template) {
    TermShmemCell * cell = (TermShmemCell*)cell_template.get();

    SetCurCellData(ch, wide_char, insert,
                   cell->GetStorage());
}

void InternalTermShmemBuffer::SetCurCellData(uint32_t ch,
                                             bool wide_char,
                                             bool insert,
                                             const CellStorage * cell_template) {
    int new_cell_count = wide_char ? 2 : 1;

    if (m_TermBuffer->m_Debug)
        std::cout << "set:[" << (char)ch << "], c:" << m_CurCol << ", r:" << m_CurRow << std::endl;

    if (!insert)
    {
        if (m_CurCol + new_cell_count > m_Cols) {
            MoveCurRow(1, true, false, cell_template);
            SetCol(0);
        }

        auto cell_storage = __GetCell(GetRow(), GetCol());

        if (!cell_storage) return;

        memmove(cell_storage, cell_template, CELL_STORAGE_SIZE);
        cell_storage->c = (wchar_t)ch;
        cell_storage->w = wide_char;
        SetCol(m_CurCol + 1);

        if (wide_char) {
            auto cell_storage = __GetCell(GetRow(), GetCol());
            memmove(cell_storage, cell_template, CELL_STORAGE_SIZE);
            cell_storage->c = (wchar_t)0;
            SetCol(m_CurCol + 1);
        }
    } else {
        uint32_t saved_row = m_CurRow;
        uint32_t saved_col = m_CurCol;

        auto line_storage = __GetLine(m_CurRow);

        auto extra_cell = ::InsertCell(line_storage, m_CurCol);

        auto cell_storage = ::GetCell(line_storage, m_CurCol);
        memmove(cell_storage, cell_template, CELL_STORAGE_SIZE);
        cell_storage->c = (wchar_t)ch;
        cell_storage->w = wide_char;
        SetCol(m_CurCol + 1);

        CellStorage * extra_cell_2 {nullptr};

        if (wide_char) {
            extra_cell_2 = ::InsertCell(line_storage, m_CurCol);

            cell_storage = ::GetCell(line_storage, m_CurCol);
            memmove(cell_storage, cell_template, CELL_STORAGE_SIZE);
            cell_storage->c = (wchar_t)0;
            SetCol(m_CurCol + 1);
        }

        if (!IsDefaultCell(extra_cell) || !IsDefaultCell(extra_cell_2)) {
            MoveCurRow(1, true, false, cell_template);
            SetCol(0);

            if (m_CurRow > saved_row)
            {
                if (!IsDefaultCell(extra_cell)) {
                    SetCurCellData((uint32_t)extra_cell->c,
                                   extra_cell->w,
                                   insert,
                                   extra_cell);
                }

                if (!IsDefaultCell(extra_cell_2)) {
                    SetCurCellData((uint32_t)extra_cell_2->c,
                                   extra_cell_2->w,
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

bool InternalTermShmemBuffer::IsDefaultCell(CellStorage * cell) {
    if (!cell)
        return true;

    if (!memcmp(EMPTY_CELL_STORAGE, cell, CELL_STORAGE_SIZE))
        return true;

    return (cell->c == ' ' &&
            cell->fore == m_TermBuffer->m_DefaultForeColorIndex &&
            cell->back == m_TermBuffer->m_DefaultBackColorIndex &&
            cell->mode == m_TermBuffer->m_DefaultMode);
}

TermLinePtr InternalTermShmemBuffer::GetLine(uint32_t row) {
    auto line_storage = __GetLine(row);

    if (line_storage) {
        auto & line_ptr = m_LinePtrs.at(row);

        if (!line_ptr)
            line_ptr = std::move(CreateTermLinePtr());
        line_ptr->SetStorage(line_storage);

        return line_ptr;
    }

    return TermLinePtr{};
}

TermCellPtr InternalTermShmemBuffer::GetCell(uint32_t row, uint32_t col) {
    auto line = GetLine(row);

    if (line) {
        return line->GetCell(col);
    }

    return TermCellPtr{};
}

LineStorage * InternalTermShmemBuffer::__GetLine(uint32_t row) {
    LineStorage * line_storage = nullptr;

    if (!m_BufferLineMapper)
        return line_storage;

    line_storage = (LineStorage *)(m_BufferLineMapper->GetLine(row));

    if (!line_storage)
        printf("invalid row:%u, rows:%u\n", row, m_Rows);

    return line_storage;
}

CellStorage * InternalTermShmemBuffer::__GetCell(uint32_t row, uint32_t col) {
    auto line = __GetLine(row);

    if (line)
        return ::GetCell(line, col);

    return nullptr;
}

void InternalTermShmemBuffer::ResetLineWithTemplate(LineStorage * line, const CellStorage * cell_template) {
    line->cols = m_Cols;
    line->last_render_index = m_Rows;
    line->modified = false;

    CellStorage * cell = (CellStorage *)(line + 1);

    for(uint32_t i=0;i < m_Cols; i++) {
        memmove(cell, cell_template, CELL_STORAGE_SIZE);
        cell++;
    }
}

void InternalTermShmemBuffer::ResetLinesWithLine(size_t begin_line,
                                                 size_t end_line,
                                                 LineStorage * line_template) {
    while (begin_line < end_line) {
        LineStorage * line = __GetLine(begin_line);

        if (line != line_template)
            memmove(line, line_template, m_LineSize);
        begin_line++;
    }
}

void InternalTermShmemBuffer::DeleteLines(uint32_t begin, uint32_t count, const TermCellPtr & cell_template) {
    TermShmemCell * cell = (TermShmemCell*)cell_template.get();

    DeleteLines(begin, count, cell->GetStorage());
}

void InternalTermShmemBuffer::DeleteLines(uint32_t begin, uint32_t count, const CellStorage * cell_template) {
    uint32_t end = m_Rows;

    if (__NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(begin,
                                                                count,
                                                                end))
    {
        return;
    }

    if (end <= begin)
        return;

    if (!m_BufferLineMapper)
        return;

    m_BufferLineMapper->RollUp(begin, end, count);

    auto clear_line_begin = end - count;

    LineStorage * clear_line = __GetLine(clear_line_begin);
    ResetLineWithTemplate(clear_line, cell_template);
    ResetLinesWithLine(clear_line_begin + 1, end, clear_line);
}

void InternalTermShmemBuffer::InsertLines(uint32_t begin, uint32_t count, const TermCellPtr & cell_template) {
    TermShmemCell * cell = (TermShmemCell*)cell_template.get();

    InsertLines(begin, count, cell->GetStorage());
}

void InternalTermShmemBuffer::InsertLines(uint32_t begin, uint32_t count, const CellStorage * cell_template) {
    uint32_t end = m_Rows;

    if (__NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(begin,
                                                                count,
                                                                end))
    {
        return;
    }

    if (end <= begin)
        return;

    if (!m_BufferLineMapper)
        return;

    m_BufferLineMapper->RollDown(begin, end, count);

    LineStorage * begin_line = __GetLine(begin);
    ResetLineWithTemplate(begin_line, cell_template);
    ResetLinesWithLine(begin + 1, begin + count, begin_line);
}

void InternalTermShmemBuffer::ScrollBuffer(int32_t scroll_offset, const TermCellPtr & cell_template) {
    TermShmemCell * cell = (TermShmemCell*)cell_template.get();

    ScrollBuffer(scroll_offset, cell->GetStorage());
}

void InternalTermShmemBuffer::ScrollBuffer(int32_t scroll_offset, const CellStorage * cell_template) {
    uint32_t begin = 0;
    uint32_t end = 0;

    if (!m_BufferLineMapper)
        return;

    if (HasScrollRegion()) {
        begin = m_ScrollRegionBegin;
        end = m_ScrollRegionEnd + 1;
    } else {
        begin = 0;
        end = m_Rows;
    }

    if (scroll_offset > 0) {
        m_BufferLineMapper->RollDown(begin, end, scroll_offset);
        LineStorage * begin_line = __GetLine(begin);

        ResetLineWithTemplate(begin_line, cell_template);
        ResetLinesWithLine(begin + 1, begin + scroll_offset, begin_line);
    } else if (scroll_offset < 0) {
        m_BufferLineMapper->RollUp(begin, end, -scroll_offset);
        size_t clear_begin = end + scroll_offset;
        LineStorage * clear_begin_line = __GetLine(clear_begin);

        ResetLineWithTemplate(clear_begin_line, cell_template);
        ResetLinesWithLine(clear_begin + 1, end, clear_begin_line);
    }
}

bool InternalTermShmemBuffer::MoveCurRow(uint32_t offset,
                                         bool move_down,
                                         bool scroll_buffer,
                                         const TermCellPtr & cell_template) {
    TermShmemCell * cell = (TermShmemCell*)cell_template.get();

    return MoveCurRow(offset, move_down, scroll_buffer, cell->GetStorage());
}

bool InternalTermShmemBuffer::MoveCurRow(uint32_t offset,
                                         bool move_down,
                                         bool scroll_buffer,
                                         const CellStorage * cell_template) {
    uint32_t begin = 0, end = m_Rows - 1;

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
    m_ScrollRegionBegin = begin;
}

void InternalTermShmemBuffer::SetScrollRegionEnd(uint32_t end) {
    m_ScrollRegionEnd = end;
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

void InternalTermShmemBuffer::__CreateBufferLineMapper() {
    auto buf = m_Storage->GetAddress();

    m_BufferLineMapper = CreateBufferLineMapper(buf, buf + m_Rows * m_LineSize,  m_LineSize);
}
