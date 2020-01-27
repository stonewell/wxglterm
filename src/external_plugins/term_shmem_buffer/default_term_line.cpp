#include "plugin_base.h"

#include "default_term_line.h"
#include "default_term_cell.h"
#include "term_buffer.h"
#include <vector>

class DefaultTermLine : public TermLine {
public:
    DefaultTermLine(TermBuffer * term_buffer) :
        PLUGIN_BASE_INIT_LIST("default_term_line", "default terminal line plugin", 0)
        , m_TermBuffer(term_buffer)
        , m_LastRenderLineIndex { (uint32_t)-1 }
    {
        (void)m_TermBuffer;
    }

	PLUGIN_BASE_DEFINE();

    void Resize(uint32_t col) override {
        Resize(col, CreateDefaultTermCell(this));
    }

    void Resize(uint32_t col, TermCellPtr cell_template) override {
        auto cur_size = m_Cells.size();

        for(auto i = cur_size; i < col; i++) {
            auto cell = CreateDefaultTermCell(this);
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
            cell = CreateDefaultTermCell(this);
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
        for(auto cell : m_Cells)
        {
            if (cell)
                cell->SetModified(modified);
        }
    }

    uint32_t GetLastRenderLineIndex() const override {
        return m_LastRenderLineIndex;
    }

    void SetLastRenderLineIndex(uint32_t index) override {
        m_LastRenderLineIndex = index;
    }
private:
    TermBuffer * m_TermBuffer;
    TermCellVector m_Cells;
    uint32_t m_LastRenderLineIndex;
};

TermLinePtr CreateDefaultTermLine(TermBuffer * term_buffer)
{
    return TermLinePtr{ new DefaultTermLine(term_buffer) };
}
