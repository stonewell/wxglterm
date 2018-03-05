#include "plugin_base.h"

#include "default_term_selection_decl.h"
#include "default_term_line.h"
#include "default_term_cell.h"

#include "internal_term_buffer.h"
#include "default_term_buffer_decl.h"

#include <vector>
#include <iostream>

class __ClonedTermBuffer : public virtual PluginBase, public virtual TermBuffer {
    __InternalTermBuffer m_Buffer;
public:
    __ClonedTermBuffer(const __InternalTermBuffer & buffer) :
        PluginBase("default_term_buffer", "default terminal buffer plugin", 0)
        , m_Buffer { buffer }
    {
    }

    MultipleInstancePluginPtr NewInstance() {
        return MultipleInstancePluginPtr{};
    }

    void Resize(uint32_t row, uint32_t col) {
        (void)row;
        (void)col;
    }

    uint32_t GetRows() {
        return m_Buffer.GetRows();
    }

    uint32_t GetCols() {
        return m_Buffer.GetCols();
    }

    uint32_t GetRow() {
        return m_Buffer.GetRow();
    }

    uint32_t GetCol() {
        return m_Buffer.GetCol();
    }

    void SetRow(uint32_t row) {
        (void)row;
    }

    void SetCol(uint32_t col) {
        (void)col;
    }

    TermLinePtr GetLine(uint32_t row) {
        return m_Buffer.GetLine(row);
    }

    TermCellPtr GetCell(uint32_t row, uint32_t col) {
        return m_Buffer.GetCell(row, col);
    }

    TermLinePtr GetCurLine() {
        return m_Buffer.GetCurLine();
    }

    TermCellPtr GetCurCell() {
        return m_Buffer.GetCurCell();
    }

    uint32_t GetScrollRegionBegin() {
        return m_Buffer.GetScrollRegionBegin();
    }

    uint32_t GetScrollRegionEnd() {
        return m_Buffer.GetScrollRegionEnd();
    }

    void SetScrollRegionBegin(uint32_t begin) {
        (void)begin;
    }

    void SetScrollRegionEnd(uint32_t end) {
        (void)end;
    }

    void DeleteLines(uint32_t begin, uint32_t count) {
        (void)begin;
        (void)count;
    }

    void InsertLines(uint32_t begin, uint32_t count) {
        (void)begin;
        (void)count;
    }

    void ScrollBuffer(int32_t scroll_offset) {
        (void)scroll_offset;
    }

    bool MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer) {
        (void)offset;
        (void)move_down;
        (void)scroll_buffer;
        return false;
    }

    void SetCellDefaults(wchar_t c,
                         uint32_t fore_color_idx,
                         uint32_t back_color_idx,
                         uint32_t mode) {
        (void)c;
        (void)fore_color_idx;
        (void)back_color_idx;
        (void)mode;
    }

    TermCellPtr CreateCellWithDefaults() {
        return TermCellPtr {};
    }

    void SetSelection(TermSelectionPtr selection) {
        (void)selection;
    }

    TermSelectionPtr GetSelection() {
        return m_Buffer.GetSelection();
    }

    void ClearSelection() {

    }

    void SetCurCellData(uint32_t ch, bool wide_char, bool insert, TermCellPtr cell_template) {
        (void)ch;
        (void)wide_char;
        (void)insert;
        (void)cell_template;
    }

    void LockUpdate() {
    }

    void UnlockUpdate() {
    }

    void EnableAlterBuffer(bool enable) {
        (void)enable;
    }

    TermBufferPtr CloneBuffer() {
        return TermBufferPtr {};
    }

    uint32_t GetMode() override {
        return m_Buffer.GetMode();
    }

    void SetMode(uint32_t m) override {
        (void)m;
    }

    void AddMode(uint32_t m) override {
        (void)m;
    }
    void RemoveMode(uint32_t m) override {
        (void)m;
    }
};

TermBufferPtr DefaultTermBuffer::CloneBuffer() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);

    return std::make_shared<__ClonedTermBuffer>(m_Buffers[m_CurBuffer]);
}
