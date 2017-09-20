#include <pybind11/embed.h>
#include "app.h"
#include "main_dlg.h"

#include "controller.h"

namespace py = pybind11;

PYBIND11_PLUGIN_IMPL(wxglterm_interface);

IMPLEMENT_APP(BatchRenameApp)

bool BatchRenameApp::OnInit()
{
    guard = std::make_shared<py::scoped_interpreter>();

    PyInit_wxglterm_interface();

    auto plugin_manager = LoadAllPlugins(".");

    MainDialog * mainDlg = new MainDialog(wxT("Batch Rename Files"));
    mainDlg->Show(true);

    return true;
}
