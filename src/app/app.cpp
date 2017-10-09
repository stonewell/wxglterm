#include <pybind11/embed.h>

#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "app.h"

#include "controller.h"
#include "wxglterm_interface.h"
#include "app_config_impl.h"

#include "term_context.h"
#include "term_network.h"
#include "term_data_handler.h"

#include "task.h"

#include "default_term_ui.h"
#include "default_term_buffer.h"

#include <iostream>
namespace py = pybind11;

wxGLTermApp::~wxGLTermApp()
{
    Cleanup();
}

bool wxGLTermApp::Run(int /*argc*/, char ** /*argv*/)
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
        PyErr_Print();
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
    std::string pluginPath(f.GetPath());

    wxFileName f2(wxStandardPaths::Get().GetExecutablePath());
    f2.AppendDir("libs");
    std::string libsPath(f2.GetPath());

    std::cout << "app path:"
              << pluginPath
              << ", libs:"
              << libsPath
              << std::endl;

    std::string plugins_dir = g_AppConfig->GetEntry("plugins/dir", pluginPath.c_str());
    std::string python_lib_dir = g_AppConfig->GetEntry("plugins/python_lib", libsPath.c_str());

    py::object sys_m = py::module::import("sys");
    sys_m.attr("path").attr("append")(pluginPath);
    sys_m.attr("path").attr("append")(libsPath);

    if (python_lib_dir != libsPath)
        sys_m.attr("path").attr("append")(python_lib_dir);

    auto plugin_manager = LoadAllPlugins(pluginPath.c_str());

    if (plugins_dir != pluginPath)
    {
        sys_m.attr("path").attr("append")(py::cast(plugins_dir));
        LoadAllPlugins(plugin_manager, plugins_dir.c_str());
    }

    m_PluginManager.swap(plugin_manager);

    InitDefaultPlugins();

    auto term_context = CreateTermContext();
    term_context->SetAppConfig(g_AppConfig);

    auto term_ui = CreateTermUI(term_context);
    auto term_network = CreateTermNetwork(term_context);
    auto term_data_handler = CreateTermDataHandler(term_context);

    if (term_context && term_ui && term_network && term_data_handler)
    {
        term_context->SetTermWindow(term_ui->CreateWindow());
        term_context->SetTermNetwork(term_network);
        term_context->SetTermDataHandler(term_data_handler);

        term_network->Connect("", 0, "", "");
        m_TermUIList.push_back(term_ui);

        auto mainwnd_task = CreateShowContextWindowTask(term_context);

        term_ui->ScheduleTask(mainwnd_task, 5, false);

        term_ui->StartMainUILoop();
        return true;
    }
    else
    {
        return false;
    }
}

TermUIPtr wxGLTermApp::CreateTermUI(TermContextPtr term_context)
{
    std::string ui_plugin_name = g_AppConfig->GetEntry("plugins/ui/name", "default_term_ui");
    uint64_t ui_plugin_version = g_AppConfig->GetEntryUInt64("plugins/ui/version", PluginManager::Latest);
    std::string ui_plugin_config = g_AppConfig->GetEntry("plugins/ui/config", "{}");

    auto plugin_term_ui = std::dynamic_pointer_cast<TermUI>(m_PluginManager->GetPlugin(ui_plugin_name.c_str(), ui_plugin_version));

    if (!plugin_term_ui)
    {
        //TODO: error or create default
        return TermUIPtr{};
    }

    std::cout << "ui plugin config:"
              << ui_plugin_config
              << std::endl;

    auto new_instance_config = CreateAppConfigFromString(ui_plugin_config.c_str());

    plugin_term_ui->InitPlugin(std::dynamic_pointer_cast<Context>(term_context),
                             new_instance_config);

    return plugin_term_ui;
}

void wxGLTermApp::InitDefaultPlugins()
{
    m_PluginManager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateDefaultTermUI()));
    m_PluginManager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateDefaultTermBuffer()));
}

TermContextPtr wxGLTermApp::CreateTermContext()
{
    std::string context_plugin_name = g_AppConfig->GetEntry("plugins/context/name", "default_term_context");
    uint64_t context_plugin_version = g_AppConfig->GetEntryUInt64("plugins/context/version", PluginManager::Latest);
    std::string context_plugin_config = g_AppConfig->GetEntry("plugins/context/config", "{}");

    auto plugin_term_context = std::dynamic_pointer_cast<TermContext>(m_PluginManager->GetPlugin(context_plugin_name.c_str(), context_plugin_version));

    if (!plugin_term_context)
    {
        //TODO: error or create default
        return TermContextPtr{};
    }

    auto new_instance = plugin_term_context->NewInstance();

    return std::dynamic_pointer_cast<TermContext>(new_instance);
}

TermNetworkPtr wxGLTermApp::CreateTermNetwork(TermContextPtr term_context)
{
    std::string plugin_name = g_AppConfig->GetEntry("plugins/network/name", "default_term_network");
    uint64_t plugin_version = g_AppConfig->GetEntryUInt64("plugins/network/version", PluginManager::Latest);
    std::string plugin_config = g_AppConfig->GetEntry("plugins/network/config", "{}");

    std::cout << "network plugin config:"
              << plugin_config
              << std::endl;

    auto plugin = std::dynamic_pointer_cast<TermNetwork>(m_PluginManager->GetPlugin(plugin_name.c_str(), plugin_version));

    if (!plugin)
    {
        //TODO: error or create default
        return TermNetworkPtr{};
    }

    auto new_instance = plugin->NewInstance();
    auto new_instance_config = CreateAppConfigFromString(plugin_config.c_str());

    new_instance->InitPlugin(std::dynamic_pointer_cast<Context>(term_context),
                             new_instance_config);

    return std::dynamic_pointer_cast<TermNetwork>(new_instance);
}

TermDataHandlerPtr wxGLTermApp::CreateTermDataHandler(TermContextPtr term_context)
{
    std::string plugin_name = g_AppConfig->GetEntry("plugins/data_handler/name", "default_term_data_handler");
    uint64_t plugin_version = g_AppConfig->GetEntryUInt64("plugins/data_handler/version", PluginManager::Latest);
    std::string plugin_config = g_AppConfig->GetEntry("plugins/data_handler/config", "{}");

    std::cout << "data_handler plugin config:"
              << plugin_config
              << std::endl;

    auto plugin = std::dynamic_pointer_cast<TermDataHandler>(m_PluginManager->GetPlugin(plugin_name.c_str(), plugin_version));

    if (!plugin)
    {
        //TODO: error or create default
        return TermDataHandlerPtr{};
    }

    auto new_instance = plugin->NewInstance();
    auto new_instance_config = CreateAppConfigFromString(plugin_config.c_str());

    new_instance->InitPlugin(std::dynamic_pointer_cast<Context>(term_context),
                             new_instance_config);

    return std::dynamic_pointer_cast<TermDataHandler>(new_instance);
}

TaskPtr wxGLTermApp::CreateShowContextWindowTask(TermContextPtr term_context)
{
    auto new_instance = ::CreateShowContextWindowTask();
    auto new_instance_config = CreateAppConfigFromString("{}");

    new_instance->InitPlugin(std::dynamic_pointer_cast<Context>(term_context),
                             new_instance_config);

    return new_instance;
}

int main(int argc, char ** argv) {
    wxGLTermApp app;

    app.Run(argc, argv);
    return 0;
}
