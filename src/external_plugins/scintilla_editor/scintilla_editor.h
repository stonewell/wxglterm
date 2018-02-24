#pragma once

#include "ScintillaBase.h"
#include "term_window.h"

class ScintillaEditor : public Scintilla::ScintillaBase {

public:
    void SetVerticalScrollPos() override;
    void SetHorizontalScrollPos() override;
    bool ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) override;
    void Copy() override;
    void Paste() override;
    void ClaimSelection() override;
    void NotifyChange() override;
    void NotifyParent(SCNotification scn) override;
    void CopyToClipboard(const Scintilla::SelectionText &selectedText) override;
    void SetMouseCapture(bool on) override;
    bool HaveMouseCapture() override;
    sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) override;
    void CreateCallTipWindow(Scintilla::PRectangle rc) override;
    void AddToPopUp(const char *label, int cmd=0, bool enabled=true) override;

public:
    TermWindow * m_pTermWindow;
    void SetTermWindow(TermWindow * pTermWindow) {
        m_pTermWindow = pTermWindow;
        wMain = this;
    }
};
