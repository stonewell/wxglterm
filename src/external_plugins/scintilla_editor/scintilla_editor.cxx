#include "plugin_base.h"

#include "scintilla_editor.h"

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

using namespace Scintilla;

ScintillaEditor::ScintillaEditor() :
    PluginBase("scintilla_editor", "scintilla editor buffer plugin", 1)
    , m_pDoc { new Document{SC_DOCUMENTOPTION_DEFAULT} }
{
    m_pDoc->AddRef();
}

ScintillaEditor::~ScintillaEditor() {
    m_pDoc->Release();
    m_pDoc = nullptr;
}

MultipleInstancePluginPtr ScintillaEditor::NewInstance() {
    return MultipleInstancePluginPtr{new ScintillaEditor()};
}

void ScintillaEditor::Resize(uint32_t row, uint32_t col) {
    (void)row;
    (void)col;
}

uint32_t ScintillaEditor::GetRows() {
    return 0;
}

uint32_t ScintillaEditor::GetCols() {
    return 0;
}

uint32_t ScintillaEditor::GetRow() {
    return 0;
}

uint32_t ScintillaEditor::GetCol() {
    return 0;
}

void ScintillaEditor::SetRow(uint32_t row) {
    (void)row;
}

void ScintillaEditor::SetCol(uint32_t col) {
    (void)col;
}

TermLinePtr ScintillaEditor::GetLine(uint32_t row) {
    (void)row;
    return TermLinePtr {};
}

TermCellPtr ScintillaEditor::GetCell(uint32_t row, uint32_t col) {
    (void)row;
    (void)col;
    return TermCellPtr {};
}

TermLinePtr ScintillaEditor::GetCurLine() {
    return TermLinePtr {};
}

TermCellPtr ScintillaEditor::GetCurCell() {
    return TermCellPtr {};
}

uint32_t ScintillaEditor::GetScrollRegionBegin() {
    return 0;
}

uint32_t ScintillaEditor::GetScrollRegionEnd() {
    return 0;
}

void ScintillaEditor::SetScrollRegionBegin(uint32_t begin) {
    (void)begin;
}

void ScintillaEditor::SetScrollRegionEnd(uint32_t end) {
    (void)end;
}

void ScintillaEditor::DeleteLines(uint32_t begin, uint32_t count) {
    (void)begin;
    (void)count;
}

void ScintillaEditor::InsertLines(uint32_t begin, uint32_t count) {
    (void)begin;
    (void)count;
}

void ScintillaEditor::ScrollBuffer(int32_t scroll_offset) {
    (void)scroll_offset;
}

bool ScintillaEditor::MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer) {
    (void)offset;
    (void)move_down;
    (void)scroll_buffer;
    return false;
}

void ScintillaEditor::SetCellDefaults(wchar_t c,
                                      uint16_t fore_color_idx,
                                      uint16_t back_color_idx,
                                      uint16_t mode) {
    (void)c;
    (void)fore_color_idx;
    (void)back_color_idx;
    (void)mode;
}

TermCellPtr ScintillaEditor::CreateCellWithDefaults() {
    return TermCellPtr {};
}

void ScintillaEditor::SetSelection(TermSelectionPtr selection) {
    (void)selection;
}

TermSelectionPtr ScintillaEditor::GetSelection() {
    return TermSelectionPtr {};
}

void ScintillaEditor::ClearSelection() {
}

void ScintillaEditor::SetCurCellData(uint32_t ch, bool wide_char, bool insert, TermCellPtr cell_template) {
    (void)ch;
    (void)wide_char;
    (void)insert;
    (void)cell_template;
}

void ScintillaEditor::LockUpdate() {
}

void ScintillaEditor::UnlockUpdate() {
}

void ScintillaEditor::EnableAlterBuffer(bool enable) {
    (void)enable;
}

uint16_t ScintillaEditor::GetMode() {
    return 0;
}

void ScintillaEditor::SetMode(uint16_t m) {
    (void)m;
}

void ScintillaEditor::AddMode(uint16_t m) {
    (void)m;
}

void ScintillaEditor::RemoveMode(uint16_t m) {
    (void)m;
}

TermBufferPtr ScintillaEditor::CloneBuffer() {
    return TermBufferPtr {};
}
