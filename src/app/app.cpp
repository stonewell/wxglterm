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

    std::cout << "termcap_dir:"
              << g_AppConfig->GetEntry("termcap_dir", "NOT FOUND!!!")
              << ","
              << g_AppConfig->GetEntry("/termcap_dir", "NOT FOUND!!! with path start with /")
              << std::endl
              << "use_system_ssh_config:" << g_AppConfig->GetEntryBool("use_system_ssh_config", false)
              << std::endl;

    std::string plugins_dir = g_AppConfig->GetEntry("plugins_dir", "NOT FOUND!!!");

    py::object sys_m = py::module::import("sys");
    sys_m.attr("path").attr("append")(plugins_dir);

    auto plugin_manager = LoadAllPlugins(plugins_dir.c_str());

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
