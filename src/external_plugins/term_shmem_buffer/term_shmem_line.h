#pragma once

#include "term_line.h"
#include <vector>

struct LineStorage {
    LineStorage()
        : modified {false} {
    }

    uint32_t cols;
    uint32_t last_render_index;
    bool modified;
};

#define LINE_STORAGE_SIZE sizeof(LineStorage)

class TermShmemLine : public TermLine {
public:
    TermShmemLine() = default;
    virtual ~TermShmemLine() = default;

public:
    virtual LineStorage * GetStorage() = 0;
    virtual void SetStorage(LineStorage * storage) = 0;
};

class TermBuffer;

using TermLineVector = std::vector<TermLinePtr>;
using TermShmemLinePtr = std::shared_ptr<TermShmemLine>;

TermShmemLinePtr CreateTermLinePtr(TermBuffer * term_buffer);
TermShmemLine * CreateRawTermLine();
