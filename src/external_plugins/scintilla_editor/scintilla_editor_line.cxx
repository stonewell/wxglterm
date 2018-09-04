#include "plugin_base.h"

#include "scintilla_editor_line.h"
#include "scintilla_editor_cell.h"
#include "term_buffer.h"
#include <vector>

class ScintillaEditorLine : public virtual PluginBase, public virtual TermLine {
public:
    ScintillaEditorLine(ScintillaEditor * pEditor, uint32_t row) :
        PluginBase("scintilla_editor_line", "default line plugin for scintilla editor", 1)
        , m_pEditor{pEditor}
        , m_Row {row}
        , m_LastRenderLineIndex {(uint32_t)-1}
    {
    }

    void Resize(uint32_t col) override {
        (void)col;
        SetModified(true);
    }

    void Resize(uint32_t col, TermCellPtr cell_template) override {
        (void)col;
        (void)cell_template;
        SetModified(true);
    }

    TermCellPtr GetCell(uint32_t col) override {
        auto cell = CreateDefaultTermCell(m_pEditor, m_Row, col);
        return cell;
    }

    //return the erased extra cell
    TermCellPtr InsertCell(uint32_t col) override {
        return GetCell(col);
    }

    bool IsModified() const override {
        return true;
    }

    void SetModified(bool modified) override {
        (void)modified;
    }

    uint32_t GetLastRenderLineIndex() const override {
        return m_LastRenderLineIndex;
    }

    void SetLastRenderLineIndex(uint32_t index) override {
        m_LastRenderLineIndex = index;
    }
private:
    ScintillaEditor * m_pEditor;
    uint32_t m_Row;
    uint32_t m_LastRenderLineIndex;
};

TermLinePtr CreateDefaultTermLine(ScintillaEditor * pEditor, int row)
{
    return TermLinePtr{ new ScintillaEditorLine(pEditor, row) };
}
