#include <pybind11/embed.h>

#include "app.h"
#include "load_module.h"

#include "controller.h"
#include "wxglterm_interface.h"
#include "app_config_impl.h"

#include "term_context.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_buffer.h"

#include "color_theme.h"
#include "input.h"

#include "task.h"

#include "default_show_context_window_task.h"

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
    m_TermUIList.clear();
    m_PluginManager.reset((PluginManager*)nullptr);
}

bool g_AppDebug = false;

static
std::string FindConfigFile() {
    try
    {
        const char *module_content =
#include "app_utils.inc"
                ;

        return LoadPyModuleFromString(module_content,
                                      "app_utils",
                                      "app_utils.py")
                .attr("find_config_file")()
                .cast<std::string>();
    }
    catch(std::exception & e)
    {
        std::cerr << "find config file failed!"
                  << std::endl
                  << e.what()
                  << std::endl;
    }
    catch(...)
    {
        std::cerr << "find config file failed!"
                  << std::endl;
        PyErr_Print();
    }

    std::cerr << "unable find the config file, use current directory wxglterm.json instead"<< std::endl;

    return "wxglterm.json";
}

class __TermContextReset {
public:
    __TermContextReset(TermContextPtr p_term_context)
        : term_context(p_term_context) {
    }

    ~__TermContextReset() {
        if (!term_context) return;
        term_context->SetTermNetwork(TermNetworkPtr{});
        term_context->SetTermWindow(TermWindowPtr{});
        term_context->SetTermDataHandler(TermDataHandlerPtr{});
        term_context->SetTermBuffer(TermBufferPtr{});
        term_context->SetTermColorTheme(TermColorThemePtr{});
        term_context->SetAppConfig(AppConfigPtr{});
        term_context->SetInputHandler(InputHandlerPtr{});
    }

    TermContextPtr term_context;
};

bool wxGLTermApp::DoInit()
{
    m_PyInterpreter = std::make_shared<py::scoped_interpreter>();

    init_wxglterm_interface_module();

    auto config_path = FindConfigFile();

    if (config_path.length() == 0)
        return false;

    g_AppConfig->LoadFromFile(config_path.c_str());

    g_AppDebug = g_AppConfig->GetEntryBool("app_debug", false);
    set_app_debug(g_AppDebug);

    if (g_AppDebug) {
        std::cout << "termcap_dir:"
                  << g_AppConfig->GetEntry("/term/termcap_dir", "NOT FOUND!!!")
                  << ","
                  << g_AppConfig->GetEntry("/term/termcap_dir", "NOT FOUND!!! with path start with /")
                  << std::endl
                  << "use_system_ssh_config:" << g_AppConfig->GetEntryBool("use_system_ssh_config", false)
                  << std::endl;
    }

    std::string pluginPath("plugins");

    std::string libsPath("libs");

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
    auto term_buffer = CreateTermBuffer(term_context);
    auto term_color_theme = CreateTermColorTheme(term_context);
    auto input_handler = CreateInputHandler(term_context);

    if (term_context
        && term_ui
        && term_network
        && term_data_handler
        && term_buffer
        && term_color_theme
        && input_handler)
    {
        std::string color_theme { g_AppConfig->GetEntry("/term/color_theme/name", "NOT FOUND") };

        if (color_theme != "NOT FOUND")
        {
            term_color_theme->Load(color_theme.c_str());
        }

        __TermContextReset context_reset{term_context};

        term_context->SetTermWindow(term_ui->CreateWindow());
        term_context->SetTermNetwork(term_network);
        term_context->SetTermDataHandler(term_data_handler);
        term_context->SetTermBuffer(term_buffer);
        term_context->SetTermColorTheme(term_color_theme);
        term_context->SetInputHandler(input_handler);

        m_TermUIList.push_back(term_ui);

        auto mainwnd_task = CreateShowContextWindowTask(term_context);

        term_ui->ScheduleTask(mainwnd_task, 5, false);

        term_ui->StartMainUILoop();

        term_network->Disconnect();
        term_data_handler->Stop();

        return true;
    }
    else
    {
        std::cerr << "fail to create plugins:"
                  << "term_ui:" << term_ui
                  << ",term_network:" << term_network
                  << ",term_data_handler:" << term_data_handler
                  << ",term_buffer:" << term_buffer
                  << ",term_color_theme:" << term_color_theme
                  << ",input_handler:" << input_handler
                  << std::endl;
        return false;
    }
}

TaskPtr wxGLTermApp::CreateShowContextWindowTask(TermContextPtr term_context)
{
    std::string plugin_name = g_AppConfig->GetEntry("plugins/show_context_window_task/name", "show_context_window_task");
    uint64_t plugin_version = g_AppConfig->GetEntryUInt64("plugins/show_context_window_task/version", PluginManager::Latest);
    std::string plugin_config = g_AppConfig->GetEntry("plugins/show_context_window_task/config", "{}");

    if (g_AppDebug)
        std::cout << "show_context_window task plugin config:"
                  << plugin_config
                  << std::endl;

    auto plugin = std::dynamic_pointer_cast<Task>(m_PluginManager->GetPlugin(plugin_name.c_str(), plugin_version));

    if (!plugin)
    {
        //TODO: error or create default
        return TaskPtr{};
    }

    auto new_instance = plugin->NewInstance();
    auto new_instance_config = CreateAppConfigFromString(plugin_config.c_str());

    new_instance->InitPlugin(std::dynamic_pointer_cast<Context>(term_context),
                             new_instance_config);

    return std::dynamic_pointer_cast<Task>(new_instance);
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

    if (g_AppDebug)
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
    m_PluginManager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateDefaultShowContextWindowTask()));
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

    if (g_AppDebug)
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

    if (g_AppDebug)
        std::cout << "config shell:"
                  << new_instance_config->GetEntry("shell", "default")
                  << std::endl;;

    return std::dynamic_pointer_cast<TermNetwork>(new_instance);
}

TermBufferPtr wxGLTermApp::CreateTermBuffer(TermContextPtr term_context)
{
    std::string plugin_name = g_AppConfig->GetEntry("plugins/buffer/name", "default_term_buffer");
    uint64_t plugin_version = g_AppConfig->GetEntryUInt64("plugins/buffer/version", PluginManager::Latest);
    std::string plugin_config = g_AppConfig->GetEntry("plugins/buffer/config", "{}");

    if (g_AppDebug)
        std::cout << "buffer plugin config:"
                  << plugin_config
                  << std::endl;

    auto plugin = std::dynamic_pointer_cast<TermBuffer>(m_PluginManager->GetPlugin(plugin_name.c_str(), plugin_version));

    if (!plugin)
    {
        //TODO: error or create default
        return TermBufferPtr{};
    }

    auto new_instance = plugin->NewInstance();
    auto new_instance_config = CreateAppConfigFromString(plugin_config.c_str());

    new_instance->InitPlugin(std::dynamic_pointer_cast<Context>(term_context),
                             new_instance_config);

    return std::dynamic_pointer_cast<TermBuffer>(new_instance);
}

TermDataHandlerPtr wxGLTermApp::CreateTermDataHandler(TermContextPtr term_context)
{
    std::string plugin_name = g_AppConfig->GetEntry("plugins/data_handler/name", "default_term_data_handler");
    uint64_t plugin_version = g_AppConfig->GetEntryUInt64("plugins/data_handler/version", PluginManager::Latest);
    std::string plugin_config = g_AppConfig->GetEntry("plugins/data_handler/config", "{}");

    if (g_AppDebug)
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

TermColorThemePtr wxGLTermApp::CreateTermColorTheme(TermContextPtr term_context)
{
    std::string plugin_name = g_AppConfig->GetEntry("plugins/color_theme/name", "default_term_color_theme");
    uint64_t plugin_version = g_AppConfig->GetEntryUInt64("plugins/color_theme/version", PluginManager::Latest);
    std::string plugin_config = g_AppConfig->GetEntry("plugins/color_theme/config", "{}");

    if (g_AppDebug)
        std::cout << "color_theme plugin config:"
                  << plugin_config
                  << std::endl;

    auto plugin = std::dynamic_pointer_cast<TermColorTheme>(m_PluginManager->GetPlugin(plugin_name.c_str(), plugin_version));

    if (!plugin)
    {
        //TODO: error or create default
        std::cerr << "unable to find color theme plguin:"
                  << plugin_name
                  << ", version:"
                  << plugin_version
                  << std::endl;

        return TermColorThemePtr{};
    }

    auto new_instance = plugin->NewInstance();
    auto new_instance_config = CreateAppConfigFromString(plugin_config.c_str());

    new_instance->InitPlugin(std::dynamic_pointer_cast<Context>(term_context),
                             new_instance_config);

    return std::dynamic_pointer_cast<TermColorTheme>(new_instance);
}

InputHandlerPtr wxGLTermApp::CreateInputHandler(TermContextPtr term_context)
{
    std::string plugin_name = g_AppConfig->GetEntry("plugins/input_handler/name", "default_term_input_handler");
    uint64_t plugin_version = g_AppConfig->GetEntryUInt64("plugins/input_handler/version", PluginManager::Latest);
    std::string plugin_config = g_AppConfig->GetEntry("plugins/input_handler/config", "{}");

    if (g_AppDebug)
        std::cout << "input handler plugin config:"
                  << plugin_config
                  << std::endl;

    auto plugin = std::dynamic_pointer_cast<InputHandler>(m_PluginManager->GetPlugin(plugin_name.c_str(), plugin_version));

    if (!plugin)
    {
        //TODO: error or create default
        std::cerr << "unable to find input handler plguin:"
                  << plugin_name
                  << ", version:"
                  << plugin_version
                  << std::endl;

        return InputHandlerPtr{};
    }

    auto new_instance = plugin->NewInstance();
    auto new_instance_config = CreateAppConfigFromString(plugin_config.c_str());

    new_instance->InitPlugin(std::dynamic_pointer_cast<Context>(term_context),
                             new_instance_config);

    return std::dynamic_pointer_cast<InputHandler>(new_instance);
}

int main(int argc, char ** argv) {
    wxGLTermApp app;

    app.Run(argc, argv);
    return 0;
}
