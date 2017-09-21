#include <pybind11/embed.h>
#include "app.h"
#include "main_dlg.h"

#include "controller.h"
#include "wxglterm_interface.h"
#include "app_config_impl.h"

#include <iostream>

namespace py = pybind11;

IMPLEMENT_APP(wxGLTermApp)

bool wxGLTermApp::OnInit()
{
    m_PyInterpreter = std::make_shared<py::scoped_interpreter>();

    init_wxglterm_interface_module();

    g_AppConfig->LoadFromFile("wxglterm.json");

    auto plugin_manager = LoadAllPlugins("../pysrc/test");

    MainDialog * mainDlg = new MainDialog(wxT("Batch Rename Files"));
    mainDlg->Show(true);

    return true;
}

int wxGLTermApp::OnExit()
{
    //must dealloc the python objects before interpreter shutdown
    g_AppConfig.reset((AppConfig*)nullptr);
    return wxApp::OnExit();
}
