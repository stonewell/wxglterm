#include "plugin_base.h"

#include "scintilla_editor_buffer.h"

#include <vector>
#include <iostream>

#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cmath>

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <cassert>
#include <functional>
#include <locale>
#include <codecvt>

#include "Platform.h"

#include "ILoader.h"
#include "ILexer.h"
#include "Scintilla.h"

#include "StringCopy.h"
#include "Position.h"
#include "UniqueString.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "XPM.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "UniConversion.h"
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"
#include "AutoComplete.h"
#include "CallTip.h"
#include "ScintillaBase.h"

#include "scintilla_editor.h"

using namespace Scintilla;

static
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcharconv;

ScintillaEditorBuffer::ScintillaEditorBuffer() :
    PluginBase("scintilla_editor", "scintilla editor buffer plugin", 1)
    , m_pEditor { new ScintillaEditor{} }
    , m_Rows {0}
    , m_Cols {0}
    , m_Row {0}
    , m_Col {0}
{
}

ScintillaEditorBuffer::~ScintillaEditorBuffer() {
    delete m_pEditor;
    m_pEditor = nullptr;
}

MultipleInstancePluginPtr ScintillaEditorBuffer::NewInstance() {
    return MultipleInstancePluginPtr{new ScintillaEditorBuffer()};
}

void ScintillaEditorBuffer::Resize(uint32_t row, uint32_t col) {
    if (m_Rows == row && m_Cols == col) {
        return;
    }

    m_Rows = row;
    m_Cols = col;

    if (m_Row >= m_Rows)
        SetRow(m_Rows ? m_Rows - 1 : 0);

    if (m_Col >= m_Cols)
        SetCol(m_Cols ? m_Cols - 1 : 0);

    ClearSelection();
}

uint32_t ScintillaEditorBuffer::GetRows() {
    return m_Rows;
}

uint32_t ScintillaEditorBuffer::GetCols() {
    return m_Cols;
}

uint32_t ScintillaEditorBuffer::GetRow() {
    return m_Row;
}

uint32_t ScintillaEditorBuffer::GetCol() {
    return m_Col;
}

void ScintillaEditorBuffer::SetRow(uint32_t row) {
    std::cout << __FUNCTION__ << ", row:" << row << std::endl;
    m_Row = row;
}

void ScintillaEditorBuffer::SetCol(uint32_t col) {
    m_Col = col;
}

uint32_t RowToLineIndex(ScintillaEditor * pEditor, uint32_t row) {
    return pEditor->WndProc(SCI_GETFIRSTVISIBLELINE, 0, 0) + row;
}

TermLinePtr ScintillaEditorBuffer::GetLine(uint32_t row) {
    auto index = RowToLineIndex(m_pEditor, row);
    auto line = CreateDefaultTermLine(m_pEditor, index);
    line->Resize(GetCols());
    return line;
}

TermCellPtr ScintillaEditorBuffer::GetCell(uint32_t row, uint32_t col) {
    TermLinePtr line = GetLine(row);

    if (line)
        return line->GetCell(col);

    return TermCellPtr{};
}

TermLinePtr ScintillaEditorBuffer::GetCurLine() {
    return GetLine(m_Row);
}

TermCellPtr ScintillaEditorBuffer::GetCurCell() {
    return GetCell(m_Row, m_Col);
}

uint32_t ScintillaEditorBuffer::GetScrollRegionBegin() {
    return 0;
}

uint32_t ScintillaEditorBuffer::GetScrollRegionEnd() {
    return 0;
}

void ScintillaEditorBuffer::SetScrollRegionBegin(uint32_t begin) {
    (void)begin;
}

void ScintillaEditorBuffer::SetScrollRegionEnd(uint32_t end) {
    (void)end;
}

void ScintillaEditorBuffer::DeleteLines(uint32_t begin, uint32_t count) {
    (void)begin;
    (void)count;
    std::cout << __FUNCTION__ << ", " << begin <<"," << count << std::endl;
}

void ScintillaEditorBuffer::InsertLines(uint32_t begin, uint32_t count) {
    (void)begin;
    (void)count;
    std::cout << __FUNCTION__ << ", " << begin <<"," << count << std::endl;
}

void ScintillaEditorBuffer::ScrollBuffer(int32_t scroll_offset) {
    (void)scroll_offset;
    std::cout << __FUNCTION__  << std::endl;
}

bool ScintillaEditorBuffer::MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer) {
    (void)offset;
    (void)move_down;
    (void)scroll_buffer;

    uint32_t line_count = m_pEditor->WndProc(SCI_GETLINECOUNT, 0, 0);

    std::cout << __FUNCTION__
              << ", offset:" << offset
              << ", move down:" << move_down
              << ", scroll_buffer:" << scroll_buffer
              << ", row:" << m_Row
              << ", line_count:" << line_count
              << std::endl;

    if (move_down) {
        if (m_Row + offset >= line_count) {
            int count = m_Row + offset - line_count;

            do {
                m_pEditor->WndProc(SCI_APPENDTEXT, 1, reinterpret_cast<sptr_t>("\n"));
                count --;
            } while (count > 0);
        }
        SetRow(m_Row + offset);
    }
    else {
        if (m_Row >= offset)
            SetRow(m_Row - offset);
        else
            SetRow(0);
    }

    return false;
}

void ScintillaEditorBuffer::SetCellDefaults(wchar_t c,
                                      uint16_t fore_color_idx,
                                      uint16_t back_color_idx,
                                      uint16_t mode) {
    m_DefaultChar = c;
    m_DefaultForeColorIndex = fore_color_idx;
    m_DefaultBackColorIndex = back_color_idx;
    m_DefaultMode = mode;
}

TermCellPtr ScintillaEditorBuffer::CreateCellWithDefaults() {
    TermCellPtr cell = CreateDefaultTermCell(nullptr, 0, 0);

    cell->SetChar(m_DefaultChar);
    cell->SetForeColorIndex(m_DefaultForeColorIndex);
    cell->SetBackColorIndex(m_DefaultBackColorIndex);
    cell->SetMode(m_DefaultMode);

    return cell;
}

void ScintillaEditorBuffer::SetSelection(TermSelectionPtr selection) {
    (void)selection;
}

TermSelectionPtr ScintillaEditorBuffer::GetSelection() {
    return TermSelectionPtr {};
}

void ScintillaEditorBuffer::ClearSelection() {
}

Sci::Position CursorToDocPos(ScintillaEditor * pEditor, uint32_t row, uint32_t col, bool relativeRow) {
    auto index = relativeRow ? RowToLineIndex(pEditor, row) : row;

    return pEditor->WndProc(SCI_POSITIONFROMLINE, index, 0) + col;
}

void ScintillaEditorBuffer::SetCurCellData(uint32_t ch,
                                           bool wide_char,
                                           bool insert,
                                           TermCellPtr cell_template) {
    (void)ch;
    (void)wide_char;
    (void)insert;
    (void)cell_template;

    auto pos = CursorToDocPos(m_pEditor, m_Row, m_Col);
    int length = m_pEditor->WndProc(SCI_GETTEXTLENGTH, 0, 0);

    std::string bytes = wcharconv.to_bytes((wchar_t)ch);

    std::cout << "row:" << m_Row << "," << m_Rows
              << ",col:" << m_Col << "," << m_Cols
              << ",pos:" << pos
              << ",length:" << length
              << ",bytes:" << bytes
              << std::endl;

    if (!insert && pos < length) {
        m_pEditor->WndProc(SCI_SETSEL, pos, pos + 1);
        m_pEditor->WndProc(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(bytes.c_str()));
    } else {
        if (pos < length)
            m_pEditor->WndProc(SCI_INSERTTEXT,
                               pos,
                               reinterpret_cast<sptr_t>(bytes.c_str()));
        else
            m_pEditor->WndProc(SCI_APPENDTEXT,
                               bytes.length(),
                               reinterpret_cast<sptr_t>(bytes.c_str()));
    }

    if (!insert)
        SetCol(m_Col + 1);

    if (m_Col >= m_Cols)
        SetRow(m_Row + 1);
}

void ScintillaEditorBuffer::LockUpdate() {
}

void ScintillaEditorBuffer::UnlockUpdate() {
}

void ScintillaEditorBuffer::EnableAlterBuffer(bool enable) {
    (void)enable;
}

uint16_t ScintillaEditorBuffer::GetMode() {
    return 0;
}

void ScintillaEditorBuffer::SetMode(uint16_t m) {
    (void)m;
}

void ScintillaEditorBuffer::AddMode(uint16_t m) {
    (void)m;
}

void ScintillaEditorBuffer::RemoveMode(uint16_t m) {
    (void)m;
}

TermBufferPtr ScintillaEditorBuffer::CloneBuffer() {
    return TermBufferPtr {};
}
