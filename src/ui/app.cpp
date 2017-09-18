#include "app.h"
#include "main_dlg.h"

IMPLEMENT_APP(BatchRenameApp)

bool BatchRenameApp::OnInit()
{

    MainDialog * mainDlg = new MainDialog(wxT("Batch Rename Files"));
    mainDlg->Show(true);

    return true;
}
