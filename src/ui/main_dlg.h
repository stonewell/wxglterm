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
    MainDialog(const wxString& title, WindowManager * winManager);

    DECLARE_EVENT_TABLE();
public:
    void RequestRefresh();
    void onClose(wxCloseEvent& evt);
private:
    DrawPane * m_DrawPane;
    WindowManager * m_WindowManager;
};
