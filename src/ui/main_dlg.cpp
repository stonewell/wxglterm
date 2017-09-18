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

    wxSizerFlags szDefaultFlags(0);
    szDefaultFlags.Expand().Border(wxTOP, 5);
    wxSizerFlags szBottomFlags(0);
    szBottomFlags.Expand().Border(wxTOP|wxBOTTOM, 5);

    //folder name
    wxStaticBoxSizer *sz = new wxStaticBoxSizer(wxVERTICAL,
                                                this,
                                                wxT("Folder for batch renaming"));

    wxBoxSizer *hboxFolder = new wxBoxSizer(wxHORIZONTAL);
    m_tcFolder = new wxTextCtrl(sz->GetStaticBox(), wxID_ANY, wxT(""));

    wxButton *folderButton = new wxButton(sz->GetStaticBox(), ID_BTN_BROWSE, wxT("Browse"));

    hboxFolder->Add(m_tcFolder,
                    wxSizerFlags(1).Expand().Border(wxBOTTOM | wxTOP, 5));
    hboxFolder->Add(folderButton, szBottomFlags);

    sz->Add(hboxFolder, szDefaultFlags);

    m_cbIncludeSubDir = new wxCheckBox(sz->GetStaticBox(), wxID_ANY, wxT("including sub directories"));
    sz->Add(m_cbIncludeSubDir, szBottomFlags);

    //match method
    wxStaticBoxSizer *sz2 = new wxStaticBoxSizer(wxVERTICAL,
                                                 this,
                                                 wxT("File name matching/replacing method"));

    m_rbWildcards = new wxRadioButton(sz2->GetStaticBox(), wxID_ANY,
                                      wxT("Use Wildcard"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);

    m_rbRegex = new wxRadioButton(sz2->GetStaticBox(), wxID_ANY,
                                  wxT("Use Regexp"));

    m_rbWildcards->SetValue(true);

    sz2->Add(m_rbWildcards, szDefaultFlags);
    sz2->Add(m_rbRegex, szDefaultFlags);
    sz2->Add(new wxStaticText(sz2->GetStaticBox(), wxID_ANY, wxT("Source file pattern")),
             szDefaultFlags);

    m_tcSourcePattern = new wxTextCtrl(sz2->GetStaticBox(), wxID_ANY, wxT("*"));
    sz2->Add(m_tcSourcePattern, szDefaultFlags);

    sz2->Add(new wxStaticText(sz2->GetStaticBox(), wxID_ANY, wxT("Target file pattern")),
             szBottomFlags);

    m_tcTargetPattern = new wxTextCtrl(sz2->GetStaticBox(), wxID_ANY, wxT("*"));
    sz2->Add(m_tcTargetPattern, szBottomFlags);

    wxStaticBoxSizer *sz3 = new wxStaticBoxSizer(wxVERTICAL,
                                                 this,
                                                 wxT("File rename preview"));

    m_lcPreview = new wxListCtrl(sz3->GetStaticBox(), wxID_ANY,
                                              wxDefaultPosition, wxDefaultSize,
                                              wxLC_REPORT | wxLC_VIRTUAL);

    wxListItem itemCol;
    itemCol.SetText(wxT("Source"));
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    m_lcPreview->InsertColumn(0, itemCol);

    itemCol.SetText(wxT("Target"));
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    m_lcPreview->InsertColumn(1, itemCol);

    m_lcPreview->SetColumnWidth( 0, wxLIST_AUTOSIZE_USEHEADER );
    m_lcPreview->SetColumnWidth( 1, wxLIST_AUTOSIZE_USEHEADER );

    sz3->Add(m_lcPreview, szBottomFlags);

    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

    hbox->Add(new wxButton(this, ID_BTN_PREVIEW, wxT("Preview Rename")),
              wxSizerFlags(1).Center());
    hbox->Add(new wxButton(this, wxID_REPLACE, wxT("Go Rename")),
              wxSizerFlags(1).Center());

    wxSizerFlags flags(szDefaultFlags);
    flags.Border(wxALL, 5);

    vbox->Add(sz, flags);
    vbox->Add(sz2, flags);
    vbox->Add(sz3, flags);
    vbox->Add(hbox, wxSizerFlags(1).Center());

    m_LogWin = (new wxLogWindow(this, "Messages", false, false));
    SetSizer(vbox);

    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");

    fileMenu->Append(ID_MENU_LOGW, "&Log window", "Open the log window");

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);

    //Connect to Events
    // Connect(wxEVT_TEXT, wxCommandEventHandler(MainDialog::OnUpdatePreview));
    // Connect(wxEVT_CHECKBOX, wxCommandEventHandler(MainDialog::OnUpdatePreview));
    // Connect(wxEVT_RADIOBUTTON, wxCommandEventHandler(MainDialog::OnUpdatePreview));
    Connect(wxEVT_BUTTON, wxCommandEventHandler(MainDialog::OnButtonClick));
    Connect(wxEVT_MOVE, wxMoveEventHandler(MainDialog::OnWindowMove));
    Connect(wxEVT_MENU, wxCommandEventHandler(MainDialog::OnMenuCommand));

    Centre();

    //Log Window
    wxLog::SetActiveTarget(m_LogWin);
    wxLog::SetLogLevel(wxLOG_Info);

    Show();

}

void MainDialog::OnUpdatePreview(wxCommandEvent & event)
{
    UpdatePreview(m_tcFolder->GetValue(),
                  m_cbIncludeSubDir->GetValue(),
                  m_rbWildcards->GetValue(),
                  m_tcSourcePattern->GetValue(),
                  m_tcTargetPattern->GetValue());

}

void MainDialog::OnButtonClick(wxCommandEvent & event)
{
    wxButton * btn = dynamic_cast<wxButton*>(event.GetEventObject());

    if (btn->GetId() == ID_BTN_BROWSE)
    {
        wxDirDialog dlg(this, wxT("Choose directory for file renaming"), wxT(""),
                        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

        dlg.SetPath(m_tcFolder->GetValue());

        if (wxID_OK == dlg.ShowModal())
        {
            m_tcFolder->SetValue(dlg.GetPath());
        }
    }
    else if (btn->GetId() == ID_BTN_PREVIEW)
    {
        UpdatePreview(m_tcFolder->GetValue(),
                      m_cbIncludeSubDir->GetValue(),
                      m_rbWildcards->GetValue(),
                      m_tcSourcePattern->GetValue(),
                      m_tcTargetPattern->GetValue());
    }
    else if (btn->GetId() == wxID_REPLACE)
    {
        //Update preview first
        if (!UpdatePreview(m_tcFolder->GetValue(),
                           m_cbIncludeSubDir->GetValue(),
                           m_rbWildcards->GetValue(),
                           m_tcSourcePattern->GetValue(),
                           m_tcTargetPattern->GetValue()))
            return;

        DoRename();
    }
}

bool MainDialog::UpdatePreview(const wxString & folder,
                               bool includeSubDir,
                               bool useWildcard,
                               const wxString & srcPattern,
                               const wxString & dstPattern)
{
    if (!wxDirExists(folder))
    {
        wxMessageBox("Please choose a valid folder!",
                     "Error",
                     wxOK | wxICON_ERROR | wxCENTRE |wxICON_HAND,
                     this);

        return false;
    }

    wxWindowDisabler disableAll;
    wxBusyInfo wait(wxT("Please wait, scanning directory..."));

    return true;
}

void MainDialog::OnWindowMove(wxMoveEvent & event)
{
    UpdateLogWindow();
}

void MainDialog::UpdateLogWindow()
{
    if (!m_LogWin)
        return;

    wxRect rc = this->GetScreenRect();
    m_LogWin->GetFrame()->Move(rc.GetLeft() + rc.GetWidth(), rc.GetTop());

    if ( m_LogWin->GetFrame()->IsIconized() )
        m_LogWin->GetFrame()->Restore();

    if ( ! m_LogWin->GetFrame()->IsShown() )
        m_LogWin->Show();

    m_LogWin->GetFrame()->SetFocus();
}

void MainDialog::OnMenuCommand(wxCommandEvent& event)
{
    if (event.GetId() == ID_MENU_LOGW)
    {
        this->UpdateLogWindow();
    }
    else if (event.GetId() == wxID_ABOUT)
    {
        wxAboutDialogInfo info;
        info.SetName(_T("Batch File Rename"));
        info.SetDescription(_T("A tool rename file in batch using wildcards or regex"));
        info.SetCopyright(_T("(C) 2017 Jingnan Si"));
        info.AddDeveloper(_T("Jingnan Si"));

        wxAboutBox(info);
    }
    else if (event.GetId() == wxID_EXIT)
    {
        Close(true);
    }
}

void MainDialog::DoRename()
{
}
