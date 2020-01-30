#pragma once

#include "default_term_line.h"
#include "term_shmem_cell.h"
#include "term_shmem_storage.h"
#include "smart_object_pool.h"

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
        return GetLine(GetRow());
    }

    TermCellPtr GetCurCell() {
        return GetCell(GetRow(), GetCol());
    }

    uint32_t GetScrollRegionBegin() const {
        return m_ScrollRegionBegin;
    }

    uint32_t GetScrollRegionEnd() const {
        return m_ScrollRegionEnd;
    }

    void SetScrollRegionBegin(uint32_t begin);
    void SetScrollRegionEnd(uint32_t end);

    void DeleteLines(uint32_t begin, uint32_t count, TermCellPtr cell_template);
    void InsertLines(uint32_t begin, uint32_t count, TermCellPtr cell_template);

    void ScrollBuffer(int32_t scroll_offset, TermCellPtr cell_template);
    bool MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer, TermCellPtr cell_template);

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

    void SetCurCellData(uint32_t ch, bool wide_char, bool insert, TermCellPtr cell_template);

    uint32_t GetMode() const;
    void SetMode(uint32_t m);
    void AddMode(uint32_t m);
    void RemoveMode(uint32_t m);

private:
    bool IsDefaultCell(TermCellPtr pcell);
    bool __NormalizeBeginEndPositionResetLinesWhenDeleteOrInsert(uint32_t & begin,
                                                                 uint32_t count,
                                                                 uint32_t & end);
    bool HasScrollRegion();
    void ClearHistoryLinesData();
    uint32_t RowToLineIndex(uint32_t row);

    TermShmemBuffer * m_TermBuffer;
    uint32_t m_Rows;
    uint32_t m_Cols;

    uint32_t m_CurRow;
    uint32_t m_CurCol;

    uint32_t m_ScrollRegionBegin;
    uint32_t m_ScrollRegionEnd;

    TermLineVector m_Lines;

    TermSelectionPtr m_Selection;

    uint32_t m_VisRowHeaderBegin;
    uint32_t m_VisRowScrollRegionBegin;
    uint32_t m_VisRowFooterBegin;
    std::bitset<16> m_Mode;

    TermShmemStoragePtr m_Storage;
    SmartObjectPool<TermShmemCell> m_TermCellPool;
    SmartObjectPool<TermLine> m_TermLinePool;
};

using InternalTermShmemBufferPtr = std::shared_ptr<InternalTermShmemBuffer>;
