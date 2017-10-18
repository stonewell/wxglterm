#include "main_dlg.h"

#include "wx/aboutdlg.h"
#include "wx/generic/aboutdlgg.h"
#include "wx/filename.h"

#include <wx/busyinfo.h>

#include <iostream>

MainDialog::MainDialog(const wxString & title)
    : wxFrame(NULL, -1, title, wxDefaultPosition, wxSize(800, 700))
{
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    wxSizerFlags flags(1);
    flags.Expand().Border(wxALL, 5);

    m_DrawPane = new DrawPane(this);

    vbox->Add(m_DrawPane, flags);

    //Connect to Events
    Centre();
}

void MainDialog::RequestRefresh()
{
    if (m_DrawPane)
        m_DrawPane->RequestRefresh();
}
