#include <pybind11/embed.h>
#include "app.h"
#include "main_dlg.h"

#include "controller.h"
#include "wxglterm_interface.h"

namespace py = pybind11;

IMPLEMENT_APP(BatchRenameApp)

bool BatchRenameApp::OnInit()
{
    guard = std::make_shared<py::scoped_interpreter>();

    init_wxglterm_interface_module();

    auto plugin_manager = LoadAllPlugins("../pysrc/test");

    MainDialog * mainDlg = new MainDialog(wxT("Batch Rename Files"));
    mainDlg->Show(true);

    return true;
}
