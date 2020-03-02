#include "plugin_base.h"

#include "term_shmem_line.h"
#include "term_shmem_cell.h"
#include "term_buffer.h"

#include "smart_object_pool.h"

#include <cstring>
#include <cassert>

static
TermShmemLine * CreateRawTermLine();

SmartObjectPool<TermShmemLine> g_TermLinePool{CreateRawTermLine};

class TermShmemLineImpl : public TermShmemLine {
public:
    TermShmemLineImpl() :
        PLUGIN_BASE_INIT_LIST("term_shmem_line", "terminal line using shared memory plugin", 0)
        , m_Storage {nullptr}
        , m_CellPtrs {}
    {
    }

	PLUGIN_BASE_DEFINE();

    void Resize(uint32_t col) override {
        m_Storage->cols = col;
        m_Storage->modified = true;
    }

    void Resize(uint32_t col, TermCellPtr cell_template) override {
        (void)cell_template;
        m_Storage->cols = col;
        m_Storage->modified = true;
    }

    TermShmemCellPtr __GetCell(uint32_t col) {
        if (col >= m_CellPtrs.size()) {
            m_CellPtrs.resize(col + 1);
        }

        auto & cell_ptr = m_CellPtrs.at(col);

        if (!cell_ptr) {
            cell_ptr = std::move(CreateTermCellPtr());
        }

        return cell_ptr;
    }

    TermCellPtr GetCell(uint32_t col) override {
        auto cell_storage = ::GetCell(m_Storage, col);

        if (!cell_storage)
            return TermCellPtr{};

        auto cell = std::move(__GetCell(col));
        cell->SetStorage(cell_storage);

        return cell;
    }

    //return the erased extra cell
    TermCellPtr InsertCell(uint32_t col) override {
        CellStorage * deleted_cell = ::InsertCell(m_Storage, col);

        if (!deleted_cell)
            return TermCellPtr{};

        auto cell = std::move(CreateTermCellPtr());
        cell->SetStorage(deleted_cell, true);

        return cell;
    }

    bool IsModified() const override {
        if (m_Storage->modified)
            return true;

        CellStorage * cell_storage_begin = (CellStorage *)(m_Storage + 1);
        CellStorage * cell_storage_end = cell_storage_begin + m_Storage->cols;

        while (cell_storage_begin < cell_storage_end) {
            if (cell_storage_begin->modified)
                return true;
            cell_storage_begin++;
        }

        return false;
    }

    void SetModified(bool modified) override {
        m_Storage->modified = modified;
    }

    uint32_t GetLastRenderLineIndex() const override {
        return m_Storage->last_render_index;
    }

    void SetLastRenderLineIndex(uint32_t index) override {
        m_Storage->last_render_index = index;
    }

    LineStorage * GetStorage() override {
        return m_Storage;
    }

    void SetStorage(LineStorage * storage) override {
        m_Storage  = storage;
    }
private:
    LineStorage * m_Storage;
    std::vector<TermShmemCellPtr> m_CellPtrs;
};

TermShmemLinePtr CreateTermLinePtr()
{
#if USE_SMART_OBJ_POOL
    return g_TermLinePool.acquire();
#else
    return std::make_shared<TermShmemLineImpl>();
#endif
}

TermShmemLine * CreateRawTermLine() {
    return new TermShmemLineImpl();
}

CellStorage * InsertCell(LineStorage * line, uint32_t col) {
    if (col >= line->cols)
        return nullptr;

    CellStorage * cell_storage_begin = (CellStorage *)(line + 1);
    CellStorage * cell_storage_end = cell_storage_begin + line->cols;

    CellStorage * cur_cell_storage = cell_storage_begin + col;
    CellStorage * next_cell_storage = cur_cell_storage + 1;
    CellStorage * last_cell_storage = cell_storage_end - 1;

    CellStorage * deleted_cell = new CellStorage;

    if (last_cell_storage->c == 0) {
        last_cell_storage--;
    }

    memcpy(deleted_cell, last_cell_storage, CELL_STORAGE_SIZE);
    memset((uint8_t*)last_cell_storage, 0, CELL_STORAGE_SIZE);

    memmove(next_cell_storage, cur_cell_storage, last_cell_storage - cur_cell_storage);

    return deleted_cell;
}

CellStorage * GetCell(LineStorage * line, uint32_t col) {
    if (col >= line->cols) {
        printf("invalid col for getcell, col:%u, cols:%u\n", col, line->cols);

        return nullptr;
    }

    CellStorage * cell_storage_begin = (CellStorage *)(line + 1);

    CellStorage * cur_cell_storage = cell_storage_begin + col;

    return cur_cell_storage;
}
