#pragma once

#include <wx/wx.h>
#include "draw_pane.h"


class MainDialog : public wxFrame
{
public:
    MainDialog(const wxString& title);

public:
    void RequestRefresh();
private:
    DrawPane * m_DrawPane;
};
