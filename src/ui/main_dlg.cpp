#include "term_window.h"
#include "main_dlg.h"

#include "wx/aboutdlg.h"
#include "wx/generic/aboutdlgg.h"
#include "wx/filename.h"

#include <wx/busyinfo.h>
#include <wx/clipbrd.h>

#include <iostream>

wxDECLARE_EVENT(MY_SET_SELECTION_EVENT, wxCommandEvent);
// this is a definition so can't be in a header
wxDEFINE_EVENT(MY_SET_SELECTION_EVENT, wxCommandEvent);

BEGIN_EVENT_TABLE(MainDialog, wxFrame)
EVT_CLOSE(MainDialog::onClose)
        EVT_COMMAND(wxID_ANY, MY_SET_SELECTION_EVENT, MainDialog::OnSetSelectionEvent)
END_EVENT_TABLE()

MainDialog::MainDialog(const wxString & title,
                       WindowManager * winManager,
                       TermWindow * termWindow)
: wxFrame(NULL, -1, title, wxDefaultPosition, wxSize(800, 700))
        , m_DrawPane(nullptr)
        , m_WindowManager(winManager)
        , m_TermWindow(termWindow)
{
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    wxSizerFlags flags(1);
    flags.Expand().Border(wxALL, 5);

    m_DrawPane = new DrawPane(this, m_TermWindow);

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

void MainDialog::SetSelectionData(const wxString & data)
{
    wxCommandEvent event(MY_SET_SELECTION_EVENT);

    event.SetString(data);
    wxPostEvent(this, event);
}

void MainDialog::OnSetSelectionEvent(wxCommandEvent& event)
{
    wxString sel_data = event.GetString();

    if (wxTheClipboard->Open()) {
        auto data_obj = new wxTextDataObject(sel_data);
        data_obj->SetFormat(wxDF_UNICODETEXT);
        wxTheClipboard->SetData(data_obj);
        wxTheClipboard->Close();
    }
}
