#pragma once

#include "term_line.h"
#include <vector>

class TermBuffer;
using TermLineVector = std::vector<TermLinePtr>;
TermLinePtr CreateDefaultTermLine(TermBuffer * term_buffer);
