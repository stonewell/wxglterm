#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>

class MainDialog : public wxFrame
{
public:
    MainDialog(const wxString& title);

private:
    enum
    {
        ID_BTN_BROWSE = wxID_HIGHEST + 1,
        ID_BTN_PREVIEW,
        ID_MENU_LOGW,
    };

    wxTextCtrl * m_tcFolder;
    wxTextCtrl * m_tcSourcePattern;
    wxTextCtrl * m_tcTargetPattern;

    wxCheckBox * m_cbIncludeSubDir;

    wxRadioButton * m_rbWildcards;
    wxRadioButton * m_rbRegex;

    wxListCtrl * m_lcPreview;

    wxLogWindow * m_LogWin;

    void OnUpdatePreview(wxCommandEvent & event);
    void OnButtonClick(wxCommandEvent & event);
    void OnWindowMove(wxMoveEvent & event);
    void OnMenuCommand(wxCommandEvent& event);

    bool UpdatePreview(const wxString & folder,
                       bool includeSubDir,
                       bool useWildcard,
                       const wxString & srcPattern,
                       const wxString & targeParttern);

    void UpdateLogWindow();
    void DoRename();
};
