#include "main_dlg.h"

#include "wx/aboutdlg.h"
#include "wx/generic/aboutdlgg.h"
#include "wx/filename.h"

#include <wx/busyinfo.h>

#include <iostream>

BEGIN_EVENT_TABLE(MainDialog, wxFrame)
EVT_CLOSE(MainDialog::onClose)
END_EVENT_TABLE()

MainDialog::MainDialog(const wxString & title, WindowManager * winManager)
: wxFrame(NULL, -1, title, wxDefaultPosition, wxSize(800, 700))
        , m_DrawPane(nullptr)
        , m_WindowManager(winManager)
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

void MainDialog::onClose(wxCloseEvent& evt)
{
    m_DrawPane = nullptr;
    m_WindowManager->WindowClosed(this);
    evt.Skip(); // don't stop event, we still want window to close
}
