#pragma once

#include "term_line.h"
#include <vector>

class ScintillaEditor;

using TermLineVector = std::vector<TermLinePtr>;
TermLinePtr CreateDefaultTermLine(ScintillaEditor * editor, int line);
