#include "plugin_base.h"

#include "term_shmem_cell.h"
#include <vector>
#include <bitset>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <string.h>

#include "smart_object_pool.h"

static
TermShmemCell * CreateRawTermCell();

SmartObjectPool<TermShmemCell> g_TermCellPool{CreateRawTermCell};

class TermShmemCellImpl : public TermShmemCell {
public:
    TermShmemCellImpl()
        : PLUGIN_BASE_INIT_LIST("term_shmem_cell", "terminal cell using shared memory plugin", 0)
        , m_Storage {nullptr}
        , m_ReleaseStorage {false}
    {
    }

    virtual ~TermShmemCellImpl() {
        if (m_ReleaseStorage && m_Storage)
            delete m_Storage;
    }

	PLUGIN_BASE_DEFINE();

public:
    CellStorage * GetStorage() override {
        return m_Storage;
    }

    void SetStorage(CellStorage * storage, bool release_storage = false) override {
        if (m_ReleaseStorage && m_Storage)
            delete m_Storage;
        m_Storage = storage;
        m_ReleaseStorage = release_storage;
    }

    wchar_t GetChar() const override {
        return m_Storage->c;
    }

    void SetChar(wchar_t c) override {
        m_Storage->c = c;
        m_Storage->modified = true;
    }

    uint32_t GetForeColorIndex() const override {
        return m_Storage->fore;
    }
    void SetForeColorIndex(uint32_t idx) override {
        m_Storage->fore = idx;
        m_Storage->modified = true;
    }
    uint32_t GetBackColorIndex() const override {
        return m_Storage->back;
    }
    void SetBackColorIndex(uint32_t idx) override {
        m_Storage->back = idx;
        m_Storage->modified = true;
    }

    uint32_t GetMode() const override {
        return m_Storage->mode;
    }

    void SetMode(uint32_t m) override {
        m_Storage->mode = m;
        m_Storage->modified = true;
    }

    void AddMode(uint32_t m) override {
        m_Storage->mode |= (1 << m);
        m_Storage->modified = true;
    }
    void RemoveMode(uint32_t m) override {
        m_Storage->mode &= ~(1 << m);
        m_Storage->modified = true;
    }

    void Reset(TermCellPtr cell) override {
        m_Storage->c = cell->GetChar();
        m_Storage->fore = cell->GetForeColorIndex();
        m_Storage->back = cell->GetBackColorIndex();
        m_Storage->mode = cell->GetMode();
        m_Storage->w = cell->IsWideChar();
        m_Storage->modified = true;
    }

    bool IsWideChar() const override {
        return m_Storage->w;
    }

    void SetWideChar(bool wide_char) override {
        m_Storage->w = wide_char;
        m_Storage->modified = true;
    }

    bool IsModified() const override {
        return m_Storage->modified;
    }

    void SetModified(bool modified) override {
        m_Storage->modified = modified;
    }

private:
    CellStorage * m_Storage;
    bool m_ReleaseStorage;
};

TermShmemCellPtr CreateTermCellPtr()
{
    return g_TermCellPool.acquire();
}

TermShmemCell * CreateRawTermCell() {
    return new TermShmemCellImpl();
}
