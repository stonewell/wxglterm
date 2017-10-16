#include "plugin_base.h"

#include "default_term_line.h"
#include "default_term_cell.h"
#include "term_buffer.h"
#include <vector>

class DefaultTermLine : public virtual PluginBase, public virtual TermLine {
public:
    DefaultTermLine(TermBuffer * term_buffer) :
        PluginBase("default_term_line", "default terminal line plugin", 0)
        , m_TermBuffer(term_buffer)
    {
    }

    void Resize(uint32_t col) override {
        m_Cells.resize(col);
    }

    TermCellPtr GetCell(uint32_t col) override {
        if (col >= m_TermBuffer->GetCols())
            return TermCellPtr{};

        auto cell = m_Cells[col];
        if (!cell)
        {
            cell = m_TermBuffer->CreateCellWithDefaults();
            m_Cells[col] = cell;
        }

        return cell;
    }

    //return the erased extra cell
    TermCellPtr InsertCell(uint32_t col) override {
        if (col >= m_TermBuffer->GetCols())
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
private:
    TermBuffer * m_TermBuffer;
    TermCellVector m_Cells;
};

TermLinePtr CreateDefaultTermLine(TermBuffer * term_buffer)
{
    return TermLinePtr{ new DefaultTermLine(term_buffer) };
}
