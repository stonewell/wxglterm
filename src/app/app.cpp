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
    bool result = false;
    try
    {
        result = DoInit();
    }
    catch(std::exception & e)
    {
        std::cerr << "!!Error Initialize:"
                  << std::endl
                  << e.what()
                  << std::endl;
    }
    catch(...)
    {
        std::cerr << "!!Error Initialize"
                  << std::endl;
        PyErr_Print();
    }

    if (!result)
    {
        Cleanup();
    }

    return result;
}

void wxGLTermApp::Cleanup()
{
    g_AppConfig.reset((AppConfig*)nullptr);
}

bool wxGLTermApp::DoInit()
{
    m_PyInterpreter = std::make_shared<py::scoped_interpreter>();

    init_wxglterm_interface_module();

    g_AppConfig->LoadFromFile("wxglterm.json");

    std::cout << "termcap_dir:"
              << g_AppConfig->GetEntry("/term/termcap_dir", "NOT FOUND!!!")
              << ","
              << g_AppConfig->GetEntry("/term/termcap_dir", "NOT FOUND!!! with path start with /")
              << std::endl
              << "use_system_ssh_config:" << g_AppConfig->GetEntryBool("use_system_ssh_config", false)
              << std::endl;

    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    f.AppendDir("plugins");
    std::string appPath(f.GetPath());

    std::cout << "app path:" << appPath << std::endl;

    std::string plugins_dir = g_AppConfig->GetEntry("plugins/dir", appPath.c_str());

    py::object sys_m = py::module::import("sys");
    sys_m.attr("path").attr("append")(appPath);

    auto plugin_manager = LoadAllPlugins(appPath.c_str());

    if (plugins_dir != appPath)
    {
        sys_m.attr("path").attr("append")(py::cast(plugins_dir));
        LoadAllPlugins(plugin_manager, plugins_dir.c_str());
    }

    m_PluginManager.swap(plugin_manager);

    auto term_ui = CreateTermUI();

    if (term_ui)
    {
        m_TermUIList.push_back(term_ui);
        term_ui->Show();

        return true;
    }
    else
    {
        return false;
    }
}

int wxGLTermApp::OnExit()
{
    //must dealloc the python objects before interpreter shutdown
    Cleanup();
    return wxApp::OnExit();
}

std::shared_ptr<TermUI> wxGLTermApp::CreateTermUI()
{
    std::string ui_plugin_name = g_AppConfig->GetEntry("plugins/ui/name", "term_ui");
    uint64_t ui_plugin_version = g_AppConfig->GetEntryUInt64("plugins/ui/version", PluginManager::Latest);

    TermUI * plugin_term_ui = dynamic_cast<TermUI*>(m_PluginManager->GetPlugin(ui_plugin_name.c_str(), ui_plugin_version));

    if (!plugin_term_ui)
    {
        //TODO: error or create default
        return std::shared_ptr<TermUI>{};
    }

    return std::shared_ptr<TermUI>{dynamic_cast<TermUI*>(plugin_term_ui->NewInstance())};
}
