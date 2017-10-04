#include "plugin_base.h"

#include "default_term_buffer.h"
#include <vector>

class DefaultTermBuffer : public virtual PluginBase, public virtual TermBuffer {
public:
    DefaultTermBuffer() :
        PluginBase("default_term_buffer", "default terminal buffer plugin", 0)
        , m_Rows(0)
        , m_Cols(0)
        , m_CurRow(0)
        , m_CurCol(0)
        , m_ScrollRegionBegin(0)
        , m_ScrollRegionEnd(0)
    {
    }

    virtual ~DefaultTermBuffer() = default;

    std::shared_ptr<MultipleInstancePlugin> NewInstance() override {
        return std::shared_ptr<MultipleInstancePlugin>{new DefaultTermBuffer()};
    }

    void Resize(uint32_t row, uint32_t col) override {
        m_Rows = row;
        m_Cols = col;

        m_Lines.resize(m_Rows);
    }

    uint32_t GetRows() const override {
        return m_Rows;
    }

    uint32_t GetCols() const override {
        return m_Cols;
    }

    uint32_t GetRow() const override {
        return m_CurRow;
    }

    uint32_t GetCol() const override {
        return m_CurCol;
    }

    TermLinePtr GetLine(uint32_t row) override {
        if (row < m_Rows)
            return m_Lines[row];

        return TermLinePtr{};
    }

    TermCellPtr GetCell(uint32_t row, uint32_t col) override {
        TermLinePtr line = GetLine(row);

        if (line)
            return line->GetCell(col);

        return TermCellPtr{};
    }

    TermLinePtr GetCurLine() override {
        return GetLine(m_CurRow);
    }

    TermCellPtr GetCurCell() override {
        return GetCell(m_CurRow, m_CurCol);
    }

    uint32_t GetScrollRegionBegin() const override {
        return m_ScrollRegionBegin;
    }

    uint32_t GetScrollRegionEnd() const override {
        return m_ScrollRegionEnd;
    }

    void SetScrollRegionBegin(uint32_t begin) override {
        m_ScrollRegionBegin = begin;
    }

    void SetScrollRegionEnd(uint32_t end) override {
        m_ScrollRegionEnd = end;
    }

    void ScrollBuffer(int32_t scroll_offset) override {
        std::vector<TermLinePtr>::iterator b_it = m_Lines.begin(),
                e_it = m_Lines.end();

        if (m_ScrollRegionBegin < m_ScrollRegionEnd) {
            b_it = b_it + m_ScrollRegionBegin;
            e_it = b_it + m_ScrollRegionEnd;
        }

        if (scroll_offset < 0) {
        }
    }
private:
    uint32_t m_Rows;
    uint32_t m_Cols;

    uint32_t m_CurRow;
    uint32_t m_CurCol;

    uint32_t m_ScrollRegionBegin;
    uint32_t m_ScrollRegionEnd;

    std::vector<TermLinePtr> m_Lines;
};
