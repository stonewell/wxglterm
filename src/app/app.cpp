#include <pybind11/embed.h>

#include <wx/filename.h>
#include <wx/stdpaths.h>

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
    try
    {
        DoInit();
        return true;
    }
    catch(std::exception & e)
    {
        std::cerr << "!!Error Initialize:"
                  << std::endl
                  << e.what()
                  << std::endl;
        return false;
    }
    catch(...)
    {
        std::cerr << "!!Error Initialize"
                  << std::endl;
        PyErr_Print();
        return false;
    }
}

void wxGLTermApp::DoInit()
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

    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    f.AppendDir("plugins");
    std::string appPath(f.GetPath());

    std::cout << "app path:" << appPath << std::endl;

    std::string plugins_dir = g_AppConfig->GetEntry("plugins_dir", appPath.c_str());

    py::object sys_m = py::module::import("sys");
    sys_m.attr("path").attr("append")(plugins_dir);

    auto plugin_manager = LoadAllPlugins(plugins_dir.c_str());

    if (plugins_dir != appPath)
    {
        sys_m.attr("path").attr("append")(py::cast(appPath));
        LoadAllPlugins(plugin_manager, appPath.c_str());
    }

    MainDialog * mainDlg = new MainDialog(wxT("Batch Rename Files"));
    mainDlg->Show(true);
}

int wxGLTermApp::OnExit()
{
    //must dealloc the python objects before interpreter shutdown
    g_AppConfig.reset((AppConfig*)nullptr);
    return wxApp::OnExit();
}
