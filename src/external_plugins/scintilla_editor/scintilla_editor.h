#pragma once

#include <mutex>
#include "plugin_base.h"
#include "term_buffer.h"

namespace Scintilla {
class Document;
};

class ScintillaEditor : public virtual PluginBase, public virtual TermBuffer {
public:
    ScintillaEditor();
    virtual ~ScintillaEditor();

    MultipleInstancePluginPtr NewInstance() override;

    void Resize(uint32_t row, uint32_t col) override;

    uint32_t GetRows() override;

    uint32_t GetCols() override;

    uint32_t GetRow() override;

    uint32_t GetCol() override;

    void SetRow(uint32_t row) override;
    void SetCol(uint32_t col) override;
    TermLinePtr GetLine(uint32_t row) override;

    TermCellPtr GetCell(uint32_t row, uint32_t col) override;

    TermLinePtr GetCurLine() override;

    TermCellPtr GetCurCell() override;

    uint32_t GetScrollRegionBegin() override;

    uint32_t GetScrollRegionEnd() override;
    void SetScrollRegionBegin(uint32_t begin) override;

    void SetScrollRegionEnd(uint32_t end) override;

    void DeleteLines(uint32_t begin, uint32_t count) override;

    void InsertLines(uint32_t begin, uint32_t count) override;

    void ScrollBuffer(int32_t scroll_offset) override;

    bool MoveCurRow(uint32_t offset, bool move_down, bool scroll_buffer) override;

    void SetCellDefaults(wchar_t c,
                         uint16_t fore_color_idx,
                         uint16_t back_color_idx,
                         uint16_t mode) override;

    TermCellPtr CreateCellWithDefaults() override;

    void SetSelection(TermSelectionPtr selection) override;
    TermSelectionPtr GetSelection() override;

    void ClearSelection() override;

    void SetCurCellData(uint32_t ch, bool wide_char, bool insert, TermCellPtr cell_template) override;

    void LockUpdate() override;

    void UnlockUpdate() override;

    void EnableAlterBuffer(bool enable) override;

    TermBufferPtr CloneBuffer() override;

    uint16_t GetMode() override;
    void SetMode(uint16_t m) override;
    void AddMode(uint16_t m) override;
    void RemoveMode(uint16_t m) override;
private:
    Scintilla::Document * m_pDoc;
};
