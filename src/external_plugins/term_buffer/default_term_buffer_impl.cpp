#include "plugin_base.h"

#include "default_term_selection_decl.h"
#include "default_term_line.h"
#include "default_term_cell.h"

#include "internal_term_buffer.h"
#include "default_term_buffer_decl.h"

#include <vector>
#include <iostream>

DefaultTermBuffer::DefaultTermBuffer() :
    PLUGIN_BASE_INIT_LIST("default_term_buffer", "default terminal buffer plugin", 1)
    , m_UpdateLock{}
    , m_DefaultChar(' ')
    , m_DefaultForeColorIndex(TermCell::DefaultForeColorIndex)
    , m_DefaultBackColorIndex(TermCell::DefaultBackColorIndex)
    , m_DefaultMode(0)
    , m_CurBuffer(0)
    , m_Buffers {{this}, {this}}
{
}

MultipleInstancePluginPtr DefaultTermBuffer::NewInstance() {
    return MultipleInstancePluginPtr{new DefaultTermBuffer()};
}

void DefaultTermBuffer::Resize(uint32_t row, uint32_t col) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);

    m_Buffers[m_CurBuffer].Resize(row, col);
}

uint32_t DefaultTermBuffer::GetRows() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetRows();
}

uint32_t DefaultTermBuffer::GetCols() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetCols();
}

uint32_t DefaultTermBuffer::GetRow() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetRow();
}

uint32_t DefaultTermBuffer::GetCol() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetCol();
}

void DefaultTermBuffer::SetRow(uint32_t row) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].SetRow(row);
}

void DefaultTermBuffer::SetCol(uint32_t col) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].SetCol(col);
}

TermLinePtr DefaultTermBuffer::GetLine(uint32_t row) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetLine(row);
}

TermCellPtr DefaultTermBuffer::GetCell(uint32_t row, uint32_t col) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetCell(row, col);
}

TermLinePtr DefaultTermBuffer::GetCurLine() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetCurLine();
}

TermCellPtr DefaultTermBuffer::GetCurCell() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetCurCell();
}

uint32_t DefaultTermBuffer::GetScrollRegionBegin() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetScrollRegionBegin();
}

uint32_t DefaultTermBuffer::GetScrollRegionEnd() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetScrollRegionEnd();
}

void DefaultTermBuffer::SetScrollRegionBegin(uint32_t begin) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].SetScrollRegionBegin(begin);
}

void DefaultTermBuffer::SetScrollRegionEnd(uint32_t end) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].SetScrollRegionEnd(end);
}

void DefaultTermBuffer::DeleteLines(uint32_t begin, uint32_t count, TermCellPtr cell_template) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].DeleteLines(begin, count, cell_template);
}

void DefaultTermBuffer::InsertLines(uint32_t begin, uint32_t count, TermCellPtr cell_template) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].InsertLines(begin, count, cell_template);
}

void DefaultTermBuffer::ScrollBuffer(int32_t scroll_offset, TermCellPtr cell_template) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].ScrollBuffer(scroll_offset, cell_template);
}

bool DefaultTermBuffer::MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer, TermCellPtr cell_template) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].MoveCurRow(offset, move_down, scroll_buffer, cell_template);
}

void DefaultTermBuffer::SetCellDefaults(wchar_t c,
                                        uint32_t fore_color_idx,
                                        uint32_t back_color_idx,
                                        uint32_t mode) {
    m_DefaultChar = c;
    m_DefaultForeColorIndex = fore_color_idx;
    m_DefaultBackColorIndex = back_color_idx;
    m_DefaultMode = mode;
}

TermCellPtr DefaultTermBuffer::CreateCellWithDefaults() {
    TermCellPtr cell = CreateDefaultTermCell(nullptr);

    cell->SetChar(m_DefaultChar);
    cell->SetForeColorIndex(m_DefaultForeColorIndex);
    cell->SetBackColorIndex(m_DefaultBackColorIndex);
    cell->SetMode(m_DefaultMode);

    return cell;
}

void DefaultTermBuffer::SetSelection(TermSelectionPtr selection) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].SetSelection(selection);
}

TermSelectionPtr DefaultTermBuffer::GetSelection() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetSelection();
}

void DefaultTermBuffer::ClearSelection() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].ClearSelection();
}

void DefaultTermBuffer::SetCurCellData(uint32_t ch, bool wide_char, bool insert, TermCellPtr cell_template) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].SetCurCellData(ch,
                                          wide_char,
                                          insert,
                                          cell_template);
}

void DefaultTermBuffer::LockUpdate() {
    m_UpdateLock.lock();
}

void DefaultTermBuffer::UnlockUpdate() {
    m_UpdateLock.unlock();
}

void DefaultTermBuffer::EnableAlterBuffer(bool enable) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);

    if (enable)
    {
        m_CurBuffer = 1;
        m_Buffers[m_CurBuffer].Resize(0, 0);
        m_Buffers[m_CurBuffer].Resize(m_Buffers[0].GetRows(), m_Buffers[0].GetCols());
    }
    else
    {
        m_CurBuffer = 0;
        m_Buffers[m_CurBuffer].Resize(m_Buffers[1].GetRows(), m_Buffers[1].GetCols());
    }
}

uint32_t DefaultTermBuffer::GetMode() {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    return m_Buffers[m_CurBuffer].GetMode();
}

void DefaultTermBuffer::SetMode(uint32_t m) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].SetMode(m);
}

void DefaultTermBuffer::AddMode(uint32_t m) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].AddMode(m);
}
void DefaultTermBuffer::RemoveMode(uint32_t m) {
    std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
    m_Buffers[m_CurBuffer].RemoveMode(m);
}
static
const std::string g_empty_str("");

void DefaultTermBuffer::SetProperty(const std::string & key, const std::string & v) {
    m_Properties.emplace(key, v);
}

const std::string & DefaultTermBuffer::GetProperty(const std::string & key) {
    auto it = m_Properties.find(key);

    if (it != m_Properties.end())
        return it->second;

    return g_empty_str;
}
