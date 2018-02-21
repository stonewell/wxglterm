#pragma once

#include "term_cell.h"
#include <vector>
#include "Position.h"

class TermLine;
class ScintillaEditor;

using TermCellVector = std::vector<TermCellPtr>;

Sci::Position CursorToDocPos(ScintillaEditor * pEditor, uint32_t row, uint32_t col, bool relativeRow = true);
uint32_t RowToLineIndex(ScintillaEditor * pEditor, uint32_t row);

TermCellPtr CreateDefaultTermCell(ScintillaEditor * pEditor, uint32_t row, uint32_t col);
