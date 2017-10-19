#pragma once

#include <wx/wx.h>

class DrawPane : public wxPanel {
public:
    DrawPane(wxFrame * parent,
             TermWindow * termWindow);
    virtual ~DrawPane();

    DECLARE_EVENT_TABLE();

public:
    void RequestRefresh();

private:
    void OnPaint(wxPaintEvent & event);
    void OnIdle(wxIdleEvent& evt);
    void OnSize(wxSizeEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void OnChar(wxKeyEvent& event);

    wxFont * GetFont();

    void DoPaint(wxDC & dc);

    int m_RefreshNow;
    wxCriticalSection m_RefreshLock;
    TermWindow * m_TermWindow;

    wxFont * m_Font;
    wxCoord m_LineHeight;
    wxCoord m_CellWidth;
};
