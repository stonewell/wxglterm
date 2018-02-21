#pragma once

#include "term_cell.h"
#include <vector>

class TermLine;
class ScintillaEditor;

using TermCellVector = std::vector<TermCellPtr>;

TermCellPtr CreateDefaultTermCell(ScintillaEditor * pEditor, uint32_t row, uint32_t col);
