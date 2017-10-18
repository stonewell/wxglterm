#pragma once

#include <wx/wx.h>
#include "draw_pane.h"


class MainDialog : public wxFrame
{
public:
    MainDialog(const wxString& title);

    DECLARE_EVENT_TABLE();
public:
    void RequestRefresh();
    void onClose(wxCloseEvent& evt);
private:
    DrawPane * m_DrawPane;
};
