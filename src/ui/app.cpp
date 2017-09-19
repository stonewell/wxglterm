#include <pybind11/embed.h>

#include "app.h"
#include "main_dlg.h"

#include "controller.h"

namespace py = pybind11;

IMPLEMENT_APP(BatchRenameApp)

bool BatchRenameApp::OnInit()
{
    py::scoped_interpreter guard{};

    auto plugin_manager = LoadAllPlugins(".");

    MainDialog * mainDlg = new MainDialog(wxT("Batch Rename Files"));
    mainDlg->Show(true);

    return true;
}
