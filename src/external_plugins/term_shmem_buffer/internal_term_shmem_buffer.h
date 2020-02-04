#pragma once

#include "term_shmem_line.h"
#include "term_shmem_cell.h"
#include "term_shmem_storage.h"

#include <bitset>

class TermShmemBuffer;

class InternalTermShmemBuffer {
public:
    InternalTermShmemBuffer(TermShmemBuffer* term_buffer);

    InternalTermShmemBuffer() = delete;
    InternalTermShmemBuffer(const InternalTermShmemBuffer & term_buffer) = delete;
    InternalTermShmemBuffer & operator = (InternalTermShmemBuffer & v) = delete;

    void Resize(uint32_t row, uint32_t col);

    uint32_t GetRows() const {
        return m_Rows;
    }

    uint32_t GetCols() const {
        return m_Cols;
    }

    uint32_t GetRow() const {
        return m_CurRow;
    }

    uint32_t GetCol() const {
        return m_CurCol;
    }

    void SetRow(uint32_t row);
    void SetCol(uint32_t col);

    TermLinePtr GetLine(uint32_t row);
    TermCellPtr GetCell(uint32_t row, uint32_t col);

    TermLinePtr GetCurLine() {
        return std::move(GetLine(GetRow()));
    }

    TermCellPtr GetCurCell() {
        return std::move(GetCell(GetRow(), GetCol()));
    }

    uint32_t GetScrollRegionBegin() const {
        return m_ScrollRegionBegin;
    }

    uint32_t GetScrollRegionEnd() const {
        return m_ScrollRegionEnd;
    }

    void SetScrollRegionBegin(uint32_t begin);
    void SetScrollRegionEnd(uint32_t end);

    void DeleteLines(uint32_t begin, uint32_t count, const TermCellPtr & cell_template);
    void InsertLines(uint32_t begin, uint32_t count, const TermCellPtr & cell_template);

    void ScrollBuffer(int32_t scroll_offset, const TermCellPtr & cell_template);
    bool MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer, const TermCellPtr & cell_template);

    void SetSelection(TermSelectionPtr selection) {
        m_Selection->SetRowBegin(selection->GetRowBegin());
        m_Selection->SetRowEnd(selection->GetRowEnd());
        m_Selection->SetColBegin(selection->GetColBegin());
        m_Selection->SetColEnd(selection->GetColEnd());
    }

    TermSelectionPtr GetSelection() {
        return m_Selection;
    }

    void ClearSelection() {
        m_Selection->SetRowBegin(0);
        m_Selection->SetRowEnd(0);
        m_Selection->SetColBegin(0);
        m_Selection->SetColEnd(0);
    }

    void SetCurCellData(uint32_t ch, bool wide_char, bool insert, const TermCellPtr & cell_template);

    uint32_t GetMode() const;
    void SetMode(uint32_t m);
    void AddMode(uint32_t m);
    void RemoveMode(uint32_t m);

private:
    void DeleteLines(uint32_t begin, uint32_t count, const CellStorage * cell_template);
    void InsertLines(uint32_t begin, uint32_t count, const CellStorage * cell_template);
    void ScrollBuffer(int32_t scroll_offset, const CellStorage * cell_template);
    bool MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer, const CellStorage * cell_template);
    void SetCurCellData(uint32_t ch, bool wide_char, bool insert, const CellStorage * cell_template);
    bool IsDefaultCell(CellStorage * pcell);
    bool __NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(uint32_t & begin,
                                                                 uint32_t count,
                                                                 uint32_t & end);
    bool HasScrollRegion();
    void ResetLineWithTemplate(LineStorage * line, const CellStorage * cell_template);
    void ResetLinesWithLine(LineStorage * begin_line,
                            LineStorage * end_line,
                            LineStorage * line_template);

    LineStorage * __GetLine(uint32_t row);
    CellStorage * __GetCell(uint32_t row, uint32_t col);

    TermShmemBuffer * m_TermBuffer;
    uint32_t m_Rows;
    uint32_t m_Cols;

    uint32_t m_CurRow;
    uint32_t m_CurCol;

    uint32_t m_ScrollRegionBegin;
    uint32_t m_ScrollRegionEnd;

    TermSelectionPtr m_Selection;

    std::bitset<16> m_Mode;

    TermShmemStoragePtr m_Storage;
};

using InternalTermShmemBufferPtr = std::shared_ptr<InternalTermShmemBuffer>;
