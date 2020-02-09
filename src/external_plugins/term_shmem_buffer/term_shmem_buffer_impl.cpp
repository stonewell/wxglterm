#include "plugin_base.h"

#include "default_term_selection_decl.h"
#include "term_shmem_cell.h"
#include "term_shmem_line.h"

#include "internal_term_shmem_buffer.h"
#include "term_shmem_buffer_decl.h"

#include <vector>
#include <iostream>

TermShmemBuffer::TermShmemBuffer() :
    PLUGIN_BASE_INIT_LIST("term_shmem_buffer", "terminal buffer plugin using shared memory as backend storage", 1)
    , m_UpdateLock{}
    , m_DefaultChar(' ')
    , m_DefaultForeColorIndex(TermCell::DefaultForeColorIndex)
    , m_DefaultBackColorIndex(TermCell::DefaultBackColorIndex)
    , m_DefaultMode(0)
    , m_CurBuffer(0)
    , m_Buffers {std::make_shared<InternalTermShmemBuffer>(this), std::make_shared<InternalTermShmemBuffer>(this)}
{
}

MultipleInstancePluginPtr TermShmemBuffer::NewInstance() {
    return MultipleInstancePluginPtr{new TermShmemBuffer()};
}

void TermShmemBuffer::Resize(uint32_t row, uint32_t col) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);

    m_Buffers[m_CurBuffer]->Resize(row, col);
}

uint32_t TermShmemBuffer::GetRows() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer]->GetRows();
}

uint32_t TermShmemBuffer::GetCols() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer]->GetCols();
}

uint32_t TermShmemBuffer::GetRow() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer]->GetRow();
}

uint32_t TermShmemBuffer::GetCol() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer]->GetCol();
}

void TermShmemBuffer::SetRow(uint32_t row) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->SetRow(row);
}

void TermShmemBuffer::SetCol(uint32_t col) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->SetCol(col);
}

TermLinePtr TermShmemBuffer::GetLine(uint32_t row) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return std::move(m_Buffers[m_CurBuffer]->GetLine(row));
}

TermCellPtr TermShmemBuffer::GetCell(uint32_t row, uint32_t col) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return std::move(m_Buffers[m_CurBuffer]->GetCell(row, col));
}

TermLinePtr TermShmemBuffer::GetCurLine() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return std::move(m_Buffers[m_CurBuffer]->GetCurLine());
}

TermCellPtr TermShmemBuffer::GetCurCell() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return std::move(m_Buffers[m_CurBuffer]->GetCurCell());
}

uint32_t TermShmemBuffer::GetScrollRegionBegin() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer]->GetScrollRegionBegin();
}

uint32_t TermShmemBuffer::GetScrollRegionEnd() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer]->GetScrollRegionEnd();
}

void TermShmemBuffer::SetScrollRegionBegin(uint32_t begin) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->SetScrollRegionBegin(begin);
}

void TermShmemBuffer::SetScrollRegionEnd(uint32_t end) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->SetScrollRegionEnd(end);
}

void TermShmemBuffer::DeleteLines(uint32_t begin, uint32_t count, const TermCellPtr & cell_template) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->DeleteLines(begin, count, cell_template);
}

void TermShmemBuffer::InsertLines(uint32_t begin, uint32_t count, const TermCellPtr & cell_template) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->InsertLines(begin, count, cell_template);
}

void TermShmemBuffer::ScrollBuffer(int32_t scroll_offset, const TermCellPtr & cell_template) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->ScrollBuffer(scroll_offset, cell_template);
}

bool TermShmemBuffer::MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer, const TermCellPtr & cell_template) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer]->MoveCurRow(offset, move_down, scroll_buffer, cell_template);
}

void TermShmemBuffer::SetCellDefaults(wchar_t c,
                                        uint32_t fore_color_idx,
                                        uint32_t back_color_idx,
                                        uint32_t mode) {
    m_DefaultChar = c;
    m_DefaultForeColorIndex = fore_color_idx;
    m_DefaultBackColorIndex = back_color_idx;
    m_DefaultMode = mode;
}

TermCellPtr TermShmemBuffer::CreateCellWithDefaults() {
    auto cell = std::move(CreateTermCellPtr());
    CellStorage * cur_cell_storage = new CellStorage;
    cell->SetStorage(cur_cell_storage, true);

    cell->SetChar(m_DefaultChar);
    cell->SetForeColorIndex(m_DefaultForeColorIndex);
    cell->SetBackColorIndex(m_DefaultBackColorIndex);
    cell->SetMode(m_DefaultMode);

    return cell;
}

void TermShmemBuffer::SetSelection(TermSelectionPtr selection) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->SetSelection(selection);
}

TermSelectionPtr TermShmemBuffer::GetSelection() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer]->GetSelection();
}

void TermShmemBuffer::ClearSelection() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->ClearSelection();
}

void TermShmemBuffer::SetCurCellData(uint32_t ch, bool wide_char, bool insert, const TermCellPtr & cell_template) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->SetCurCellData(ch,
                                          wide_char,
                                          insert,
                                          cell_template);
}

void TermShmemBuffer::LockUpdate() {
    m_UpdateLock.lock();
}

void TermShmemBuffer::UnlockUpdate() {
    m_UpdateLock.unlock();
}

void TermShmemBuffer::EnableAlterBuffer(bool enable) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);

    if (enable)
    {
        m_CurBuffer = 1;
        m_Buffers[m_CurBuffer]->Resize(m_Buffers[0]->GetRows(), m_Buffers[0]->GetCols());
    }
    else
    {
        m_CurBuffer = 0;
        m_Buffers[m_CurBuffer]->Resize(m_Buffers[1]->GetRows(), m_Buffers[1]->GetCols());
    }
}

uint32_t TermShmemBuffer::GetMode() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer]->GetMode();
}

void TermShmemBuffer::SetMode(uint32_t m) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->SetMode(m);
}

void TermShmemBuffer::AddMode(uint32_t m) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->AddMode(m);
}

void TermShmemBuffer::RemoveMode(uint32_t m) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer]->RemoveMode(m);
}

TermBufferPtr TermShmemBuffer::CloneBuffer() {
    return TermBufferPtr{};
}

static
const std::string g_empty_str("");

void TermShmemBuffer::SetProperty(const std::string & key, const std::string & v) {
    m_Properties.emplace(key, v);
}

const std::string & TermShmemBuffer::GetProperty(const std::string & key) {
    auto it = m_Properties.find(key);

    if (it != m_Properties.end())
        return it->second;

    return g_empty_str;
}
