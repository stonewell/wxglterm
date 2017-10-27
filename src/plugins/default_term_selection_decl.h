#pragma once

#include "plugin_base.h"
#include "term_selection.h"

class DefaultTermSelection : public virtual PluginBase, public virtual TermSelection {
public:
    DefaultTermSelection() :
        PluginBase("default_term_buffer_cpp", "default terminal buffer plugin", 1)
        , m_RowBegin{0}
        , m_ColBegin(0)
        , m_RowEnd{0}
        , m_ColEnd(0)
    {
    }

    virtual ~DefaultTermSelection() = default;

    uint32_t GetRowBegin() const override {
        return m_RowBegin;
    }

    void SetRowBegin(uint32_t rowBegin) override {
        m_RowBegin = rowBegin;
    }
    uint32_t GetColBegin() const override {
        return m_ColBegin;
    }

    void SetColBegin(uint32_t colBegin) override {
        m_ColBegin = colBegin;
    }

    uint32_t GetRowEnd() const override {
        return m_RowEnd;
    }
    void SetRowEnd(uint32_t rowEnd) override {
        m_RowEnd = rowEnd;
    }
    uint32_t GetColEnd() const override {
        return m_ColEnd;
    }
    void SetColEnd(uint32_t colEnd) override {
        m_ColEnd = colEnd;
    }
private:
    uint32_t m_RowBegin, m_ColBegin;
    uint32_t m_RowEnd, m_ColEnd;
};
