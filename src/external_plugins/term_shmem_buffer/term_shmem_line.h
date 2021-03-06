#pragma once

#include "term_line.h"
#include <vector>

struct LineStorage {
    uint32_t cols;
    uint32_t last_render_index;
    bool modified;
};

#define LINE_STORAGE_SIZE sizeof(LineStorage)

struct CellStorage;

CellStorage * InsertCell(LineStorage * line, uint32_t col);
CellStorage * GetCell(LineStorage * line, uint32_t col);

class TermShmemLine : public TermLine {
public:
    TermShmemLine() = default;
    virtual ~TermShmemLine() = default;

public:
    virtual LineStorage * GetStorage() = 0;
    virtual void SetStorage(LineStorage * storage) = 0;
};

using TermShmemLinePtr = std::shared_ptr<TermShmemLine>;

TermShmemLinePtr CreateTermLinePtr();
