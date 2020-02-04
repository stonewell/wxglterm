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
{
}

void InternalTermShmemBuffer::Resize(uint32_t row, uint32_t col) {
    if (m_Rows == row && m_Cols == col) {
        return;
    }

    size_t storage_size = (LINE_STORAGE_SIZE + col * CELL_STORAGE_SIZE) * row;
    size_t line_size = LINE_STORAGE_SIZE + CELL_STORAGE_SIZE * col;

    m_Storage = CreateTermShmemStorage(storage_size);

    auto buf = m_Storage->GetAddress();
    for(uint32_t i=0;i<row;i++) {
        LineStorage * line = (LineStorage *)(buf + line_size * i);

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

    auto buf = m_Storage->GetAddress();

    size_t line_size = LINE_STORAGE_SIZE + CELL_STORAGE_SIZE * GetCols();

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
        LineStorage * begin_line = (LineStorage *)(buf + line_size * begin);
        LineStorage * end_line = (LineStorage *)(buf + line_size * end);

        memset((uint8_t*)begin_line, 0, BYTES_COUNT(end_line, begin_line));

        //Reset line directly
        for (uint32_t i = begin;i < end; i++)
        {
            begin_line->cols = GetCols();
            begin_line->last_render_index = GetRows();
            begin_line->modified = false;

            begin_line = (LineStorage *)(((uint8_t*)begin_line) + line_size);
        }
    }

    return reset_lines;
}

void InternalTermShmemBuffer::SetCurCellData(uint32_t ch,
                                             bool wide_char,
                                             bool insert,
                                             const TermCellPtr & cell_template) {
    auto cell = std::dynamic_pointer_cast<TermShmemCell>(cell_template);

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
        auto line = std::move(CreateTermLinePtr());
        line->SetStorage(line_storage);

        return line;
    }

    return TermLinePtr{};
}

TermCellPtr InternalTermShmemBuffer::GetCell(uint32_t row, uint32_t col) {
    auto cell_storage = __GetCell(row, col);

    if (cell_storage) {
        auto cell = std::move(CreateTermCellPtr());
        cell->SetStorage(cell_storage);

        return cell;
    }

    return TermCellPtr{};
}

LineStorage * InternalTermShmemBuffer::__GetLine(uint32_t row) {
    if (row < GetRows()) {
        auto buf = m_Storage->GetAddress();
        size_t line_size = LINE_STORAGE_SIZE + CELL_STORAGE_SIZE * GetCols();
        LineStorage * line_storage = (LineStorage *)(buf + line_size * row);

        return line_storage;
    }

    printf("invalid row:%u, rows:%u\n", row, GetRows());
    assert(false);
    return nullptr;
}

CellStorage * InternalTermShmemBuffer::__GetCell(uint32_t row, uint32_t col) {
    auto line = __GetLine(row);

    if (line)
        return ::GetCell(line, col);

    return nullptr;
}

void InternalTermShmemBuffer::ResetLineWithTemplate(LineStorage * line, const CellStorage * cell_template) {
    //create a default line then copy line to the left lines
    line->cols = GetCols();
    line->last_render_index = GetRows();
    line->modified = false;

    CellStorage * cell = (CellStorage *)(line + 1);

    for(uint32_t i=0;i < GetCols(); i++) {
        memmove(cell, cell_template, CELL_STORAGE_SIZE);
        cell++;
    }
}

void InternalTermShmemBuffer::ResetLinesWithLine(LineStorage * begin_line,
                        LineStorage * end_line,
                        LineStorage * line_template) {
    size_t line_size = LINE_STORAGE_SIZE + CELL_STORAGE_SIZE * GetCols();

    while (begin_line < end_line) {
        memmove(begin_line, line_template, line_size);
        begin_line = (LineStorage *)(((uint8_t *)begin_line) + line_size);
    }
}

void InternalTermShmemBuffer::DeleteLines(uint32_t begin, uint32_t count, const TermCellPtr & cell_template) {
    auto cell = std::dynamic_pointer_cast<TermShmemCell>(cell_template);

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

    auto buf = m_Storage->GetAddress();
    size_t line_size = LINE_STORAGE_SIZE + CELL_STORAGE_SIZE * GetCols();

    LineStorage * begin_line = (LineStorage *)(buf + line_size * begin);
    LineStorage * end_line = (LineStorage *)(buf + line_size * end);
    LineStorage * copy_start_line = (LineStorage *)(buf + line_size * (begin + count));

    memmove(begin_line, copy_start_line, end_line - copy_start_line);

    LineStorage * copy_end_line = (LineStorage *)(((uint8_t*)begin_line) + BYTES_COUNT(end_line, copy_start_line));

    ResetLineWithTemplate(copy_end_line, cell_template);

    ResetLinesWithLine(copy_end_line, end_line, copy_end_line);
}

void InternalTermShmemBuffer::InsertLines(uint32_t begin, uint32_t count, const TermCellPtr & cell_template) {
    auto cell = std::dynamic_pointer_cast<TermShmemCell>(cell_template);

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

    auto buf = m_Storage->GetAddress();
    size_t line_size = LINE_STORAGE_SIZE + CELL_STORAGE_SIZE * GetCols();

    LineStorage * begin_line = (LineStorage *)(buf + line_size * begin);
    LineStorage * end_line = (LineStorage *)(buf + line_size * end);
    LineStorage * copy_end_line = (LineStorage *)(buf + line_size * (end - count));
    LineStorage * dest_begin_line = (LineStorage *)(((uint8_t*)end_line) - BYTES_COUNT(copy_end_line, begin_line));

    memmove(dest_begin_line, begin_line, BYTES_COUNT(copy_end_line, begin_line));

    ResetLineWithTemplate(begin_line, cell_template);
    ResetLinesWithLine(begin_line, dest_begin_line, begin_line);
}

void InternalTermShmemBuffer::ScrollBuffer(int32_t scroll_offset, const TermCellPtr & cell_template) {
    auto cell = std::dynamic_pointer_cast<TermShmemCell>(cell_template);

    ScrollBuffer(scroll_offset, cell->GetStorage());
}

void InternalTermShmemBuffer::ScrollBuffer(int32_t scroll_offset, const CellStorage * cell_template) {
    uint32_t begin = 0;
    uint32_t end = 0;

    if (HasScrollRegion()) {
        begin = m_ScrollRegionBegin;
        end = m_ScrollRegionEnd + 1;
    } else {
        begin = 0;
        end = m_Rows;
    }

    auto buf = m_Storage->GetAddress();
    size_t line_size = LINE_STORAGE_SIZE + CELL_STORAGE_SIZE * GetCols();
    LineStorage * begin_line = (LineStorage *)(buf + line_size * begin);
    LineStorage * end_line = (LineStorage *)(buf + line_size * end);

    if (scroll_offset > 0) {
        LineStorage * dest_begin_line = (LineStorage *)(buf + line_size * (begin + scroll_offset));

        memmove(dest_begin_line, begin_line, BYTES_COUNT(end_line, dest_begin_line));

        ResetLineWithTemplate(begin_line, cell_template);
        ResetLinesWithLine(begin_line, dest_begin_line, begin_line);
    } else if (scroll_offset < 0) {
        LineStorage * copy_begin_line = (LineStorage *)(buf + line_size * (begin - scroll_offset));
        LineStorage * clear_begin_line = (LineStorage *)(((uint8_t*)begin_line) +
                                                         BYTES_COUNT(end_line, copy_begin_line));

        memmove(begin_line, copy_begin_line, BYTES_COUNT(end_line, copy_begin_line));

        ResetLineWithTemplate(clear_begin_line, cell_template);
        ResetLinesWithLine(clear_begin_line, end_line, clear_begin_line);
    }
}

bool InternalTermShmemBuffer::MoveCurRow(uint32_t offset,
                                         bool move_down,
                                         bool scroll_buffer,
                                         const TermCellPtr & cell_template) {
    auto cell = std::dynamic_pointer_cast<TermShmemCell>(cell_template);

    return MoveCurRow(offset, move_down, scroll_buffer, cell->GetStorage());
}

bool InternalTermShmemBuffer::MoveCurRow(uint32_t offset,
                                         bool move_down,
                                         bool scroll_buffer,
                                         const CellStorage * cell_template) {
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
