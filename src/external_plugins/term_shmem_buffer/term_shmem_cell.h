#pragma once

#include "term_cell.h"
#include <vector>

struct CellStorage {
    CellStorage()
        : modified {false} {
    }

    wchar_t c;
    uint32_t fore;
    uint32_t back;
    uint32_t mode;
    bool w;
    bool modified;
};

#define CELL_STORAGE_SIZE sizeof(CellStorage)

class TermShmemCell : public TermCell {
public:
    TermShmemCell() = default;
    virtual ~TermShmemCell() = default;

    virtual CellStorage * GetStorage() = 0;
    virtual void SetStorage(CellStorage * storage, bool release_storage = false) = 0;
};

using TermShmemCellPtr = std::shared_ptr<TermShmemCell>;

TermShmemCellPtr CreateTermCellPtr();
