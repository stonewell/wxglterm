#pragma once

#include "term_cell.h"
#include <vector>

class TermLine;

using TermCellVector = std::vector<TermCellPtr>;

TermCellPtr CreateDefaultTermCell(TermLine * line);
