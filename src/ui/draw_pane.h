#pragma once

#include <wx/wx.h>

class DrawPane : public wxPanel {
public:
    DrawPane(wxFrame * parent);
    virtual ~DrawPane();

    DECLARE_EVENT_TABLE();

public:
    void RequestRefresh();

private:
    void PaintEvent(wxPaintEvent & event);
    void DoPaint(wxDC & dc);
    void OnIdle(wxIdleEvent& evt);

    int m_RefreshNow;
    wxCriticalSection m_RefreshLock;
};
