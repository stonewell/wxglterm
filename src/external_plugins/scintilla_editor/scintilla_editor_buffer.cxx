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
    m_Rows = row;
    m_Cols = col;
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
    m_Row = row;
}

void ScintillaEditorBuffer::SetCol(uint32_t col) {
    m_Col = col;
}

TermLinePtr ScintillaEditorBuffer::GetLine(uint32_t row) {
    (void)row;
    return TermLinePtr {};
}

TermCellPtr ScintillaEditorBuffer::GetCell(uint32_t row, uint32_t col) {
    (void)row;
    (void)col;
    return TermCellPtr {};
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
}

void ScintillaEditorBuffer::InsertLines(uint32_t begin, uint32_t count) {
    (void)begin;
    (void)count;
}

void ScintillaEditorBuffer::ScrollBuffer(int32_t scroll_offset) {
    (void)scroll_offset;
}

bool ScintillaEditorBuffer::MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer) {
    (void)offset;
    (void)move_down;
    (void)scroll_buffer;
    return false;
}

void ScintillaEditorBuffer::SetCellDefaults(wchar_t c,
                                      uint16_t fore_color_idx,
                                      uint16_t back_color_idx,
                                      uint16_t mode) {
    (void)c;
    (void)fore_color_idx;
    (void)back_color_idx;
    (void)mode;
}

TermCellPtr ScintillaEditorBuffer::CreateCellWithDefaults() {
    return TermCellPtr {};
}

void ScintillaEditorBuffer::SetSelection(TermSelectionPtr selection) {
    (void)selection;
}

TermSelectionPtr ScintillaEditorBuffer::GetSelection() {
    return TermSelectionPtr {};
}

void ScintillaEditorBuffer::ClearSelection() {
}

void ScintillaEditorBuffer::SetCurCellData(uint32_t ch, bool wide_char, bool insert, TermCellPtr cell_template) {
    (void)ch;
    (void)wide_char;
    (void)insert;
    (void)cell_template;
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
