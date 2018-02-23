#pragma once

#include <wx/wx.h>
#include "draw_pane.h"

class WindowManager {
public:
    WindowManager() = default;
    virtual ~WindowManager() = default;

public:
    virtual void WindowClosed(wxFrame * win) = 0;
};

class MainDialog : public wxFrame
{
public:
    MainDialog(const wxString& title,
               WindowManager * winManager,
               TermWindow * termWindow);

    DECLARE_EVENT_TABLE();

public:
    void RequestRefresh();
    void onClose(wxCloseEvent& evt);
    uint32_t GetColorByIndex(uint32_t index) {
        return m_DrawPane->GetColorByIndex(index);
    }

    void SetSelectionData(const wxString & data);
    void EnableMouseTrack(bool enable) {
        m_DrawPane->EnableMouseTrack(enable);
    }

    uint32_t GetWidth() {
        return m_DrawPane->GetWidth();
    }

    uint32_t GetHeight() {
        return m_DrawPane->GetHeight();
    }

    uint32_t GetLineHeight() {
        return m_DrawPane->GetLineHeight();
    }

    uint32_t GetColWidth() {
        return m_DrawPane->GetColWidth();
    }
private:
    DrawPane * m_DrawPane;
    WindowManager * m_WindowManager;
    TermWindow * m_TermWindow;

    void OnSetSelectionEvent(wxCommandEvent& event);
};
