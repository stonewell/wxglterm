#include "plugin_base.h"

#include "term_shmem_line.h"
#include "term_shmem_cell.h"
#include "term_buffer.h"

class TermShmemLineImpl : public TermShmemLine {
public:
    TermShmemLineImpl(TermBuffer * term_buffer) :
        PLUGIN_BASE_INIT_LIST("term_shmem_line", "terminal line using shared memory plugin", 0)
        , m_TermBuffer(term_buffer)
        , m_Storage {nullptr}
    {
        (void)m_TermBuffer;
    }

	PLUGIN_BASE_DEFINE();

    void Resize(uint32_t col) override {
        m_Storage->cols = col;
        m_Storage->modified = true;
    }

    void Resize(uint32_t col, TermCellPtr cell_template) override {
        auto cur_size = m_Cells.size();

        for(auto i = cur_size; i < col; i++) {
            auto cell = CreateTermCellPtr();
            cell->Reset(cell_template);
            cell->SetChar(L' ');
            m_Cells.push_back(cell);
        }

        SetModified(true);
    }

    TermCellPtr GetCell(uint32_t col) override {
        if (col >= m_Cells.size()) {
            printf("invalid col for getcell, col:%u, cols:%zu\n", col, m_Cells.size());

            return TermCellPtr{};
        }

        auto cell = m_Cells[col];
        if (!cell)
        {
            cell = CreateTermCellPtr();
            m_Cells[col] = cell;
        }

        return cell;
    }

    //return the erased extra cell
    TermCellPtr InsertCell(uint32_t col) override {
        if (col >= m_Cells.size())
            return TermCellPtr{};

        TermCellVector::iterator it = m_Cells.begin() + col;
        m_Cells.insert(it, TermCellPtr{});

        it = m_Cells.end() - 1;

        TermCellPtr cell = *it;

        if (cell && cell->GetChar() == 0) {
            it--;
            cell = *it;

            *it = TermCellPtr{};
        }

        m_Cells.erase(m_Cells.end() - 1);

        return cell;
    }

    bool IsModified() const override {
        for(auto cell : m_Cells)
        {
            if (cell && cell->IsModified())
                return true;
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
    TermBuffer * m_TermBuffer;
    LineStorage * m_Storage;
};

TermShmemLinePtr CreateTermLinePtr(TermBuffer * term_buffer)
{
    return TermShmemLinePtr{ new TermShmemLine(term_buffer) };
}
