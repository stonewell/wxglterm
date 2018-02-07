#include <iostream>
#include <sstream>

#ifdef WXGLTERM_DYNAMIC_INTERFACE
#include <pybind11/pybind11.h>
#else
#include <pybind11/embed.h>
#endif
#include <pybind11/stl.h>

#include "py_term_cell.h"
#include "py_term_line.h"
#include "py_term_selection.h"
#include "py_term_buffer.h"
#include "py_term_window.h"
#include "py_task.h"
#include "py_term_ui.h"
#include "py_term_network.h"
#include "py_term_data_handler.h"
#include "py_color_theme.h"
#include "py_input.h"

#include "py_term_context.h"

#include "py_plugin_manager.h"

#include "py_multiple_instance_plugin.h"

#include "py_app_config.h"

#include "wxglterm_interface.h"

namespace py = pybind11;

void print_plugin_info(Plugin * plugin)
{
    std::cerr << "Name:" << plugin->GetName()
              << ", Description:" << plugin->GetDescription()
              << ", Version:" << plugin->GetVersion()
              << std::endl;
}

#ifdef WXGLTERM_DYNAMIC_INTERFACE
PYBIND11_MODULE(wxglterm_interface, m)
#else
PYBIND11_EMBEDDED_MODULE(wxglterm_interface, m)
#endif
{
    m.def("print_plugin_info", &print_plugin_info);

    py::class_<Plugin, PyPlugin<>, std::shared_ptr<Plugin>> plugin(m, "Plugin");
    plugin.def(py::init<>())
            .def("get_name", &Plugin::GetName)
            .def("get_description", &Plugin::GetDescription)
            .def("get_version", &Plugin::GetVersion)
            .def("init_plugin", &Plugin::InitPlugin)
            .def("get_plugin_context", &Plugin::GetPluginContext)
            .def("get_plugin_config", &Plugin::GetPluginConfig)
            ;

    py::class_<MultipleInstancePlugin, PyMultipleInstancePlugin<>, std::shared_ptr<MultipleInstancePlugin>> multiple_instance_plugin(m, "MultipleInstancePlugin", plugin);
    multiple_instance_plugin.def(py::init<>())
            .def("new_instance", &MultipleInstancePlugin::NewInstance);

    py::class_<Context, PyContext<>, std::shared_ptr<Context>> context(m, "Context", multiple_instance_plugin);
    context.def(py::init<>())
            .def_property("app_config", &Context::GetAppConfig, &Context::SetAppConfig)
            ;

    py::class_<TermBuffer, PyTermBuffer<>, std::shared_ptr<TermBuffer>> term_buffer(m, "TermBuffer", multiple_instance_plugin);
    term_buffer.def(py::init<>())
            .def("resize", &TermBuffer::Resize)
            .def_property_readonly("rows", &TermBuffer::GetRows)
            .def_property_readonly("cols", &TermBuffer::GetCols)
            .def_property("row", &TermBuffer::GetRow, &TermBuffer::SetRow)
            .def_property("col", &TermBuffer::GetCol, &TermBuffer::SetCol)
            .def("get_line", &TermBuffer::GetLine)
            .def("get_cell", &TermBuffer::GetCell)
            .def_property_readonly("cur_line", &TermBuffer::GetCurLine)
            .def_property_readonly("cur_cell", &TermBuffer::GetCurCell)
            .def("scroll_buffer", &TermBuffer::ScrollBuffer)
            .def_property("scroll_region_begin", &TermBuffer::GetScrollRegionBegin, &TermBuffer::SetScrollRegionBegin)
            .def_property("scroll_region_end", &TermBuffer::GetScrollRegionEnd, &TermBuffer::SetScrollRegionEnd)
            .def("delete_lines", &TermBuffer::DeleteLines)
            .def("insert_lines", &TermBuffer::InsertLines)
            .def("set_cell_defaults", &TermBuffer::SetCellDefaults)
            .def("create_cell_with_defaults", &TermBuffer::CreateCellWithDefaults)
            .def_property("selection", &TermBuffer::GetSelection, &TermBuffer::SetSelection)
            .def("clear_selection", &TermBuffer::ClearSelection)
            .def("move_cur_row", &TermBuffer::MoveCurRow)
            .def("set_cur_cell_data", &TermBuffer::SetCurCellData)
            .def("lock_update", &TermBuffer::LockUpdate)
            .def("unlock_update", &TermBuffer::UnlockUpdate)
            .def("enable_alter_buffer", &TermBuffer::EnableAlterBuffer)
            .def("clone_buffer", &TermBuffer::CloneBuffer)
            .def_property("mode", &TermBuffer::GetMode, &TermBuffer::SetMode)
            .def("add_mode", &TermBuffer::AddMode)
            .def("remove_mode", &TermBuffer::RemoveMode)
            ;

    py::class_<TermLine, PyTermLine<>, std::shared_ptr<TermLine>> term_line(m, "TermLine", plugin);
    term_line.def(py::init<>())
            .def("resize", &TermLine::Resize)
            .def("get_cell", &TermLine::GetCell)
            .def("insert_cell", &TermLine::InsertCell)
            .def_property("is_modified", &TermLine::IsModified, &TermLine::SetModified)
            .def_property("last_render_line_index", &TermLine::GetLastRenderLineIndex, &TermLine::SetLastRenderLineIndex)
            ;

    py::class_<TermCell, PyTermCell<>, std::shared_ptr<TermCell>> term_cell(m, "TermCell", plugin);
    term_cell.def(py::init<>())
            .def_property("char", &TermCell::GetChar, &TermCell::SetChar)
            .def_property("fore_color_idx", &TermCell::GetForeColorIndex, &TermCell::SetForeColorIndex)
            .def_property("back_color_idx", &TermCell::GetBackColorIndex, &TermCell::SetBackColorIndex)
            .def_property("mode", &TermCell::GetMode, &TermCell::SetMode)
            .def("add_mode", &TermCell::AddMode)
            .def("remove_mode", &TermCell::RemoveMode)
            .def("reset", &TermCell::Reset)
            .def_property("is_wide_char", &TermCell::IsWideChar, &TermCell::SetWideChar)
            .def_property("is_modified", &TermCell::IsModified, &TermCell::SetModified)
            .def("__repr__",
                 [](const TermCell &a) {
                     std::stringstream ss;

                     ss << "char:" << a.GetChar() << ", mode:" << a.GetMode() << ", fore:" << a.GetForeColorIndex() << ", back:" << a.GetBackColorIndex();

                     return ss.str();
                 }
                 );


    py::class_<TermSelection, PyTermSelection<>, std::shared_ptr<TermSelection>> term_selection(m, "TermSelection", plugin);
    term_selection.def(py::init<>())
            .def_property("row_begin", &TermSelection::GetRowBegin, &TermSelection::SetRowBegin)
            .def_property("col_begin", &TermSelection::GetColBegin, &TermSelection::SetColBegin)
            .def_property("row_end", &TermSelection::GetRowEnd, &TermSelection::SetRowEnd)
            .def_property("col_end", &TermSelection::GetColEnd, &TermSelection::SetColEnd)
            ;

    py::enum_<TermCell::ColorIndexEnum>(term_cell, "ColorIndex")
            .value("DefaultForeColorIndex", TermCell::ColorIndexEnum::DefaultForeColorIndex)
            .value("DefaultBackColorIndex", TermCell::ColorIndexEnum::DefaultBackColorIndex)
            .value("DefaultCursorColorIndex", TermCell::ColorIndexEnum::DefaultCursorColorIndex)
            .export_values();
    py::enum_<TermCell::TextModeEnum>(term_cell, "TextMode")
            .value("Stdout", TermCell::TextModeEnum::Stdout)
            .value("Reverse", TermCell::TextModeEnum::Reverse)
            .value("Selection", TermCell::TextModeEnum::Selection)
            .value("Cursor", TermCell::TextModeEnum::Cursor)
            .value("Bold", TermCell::TextModeEnum::Bold)
            .value("Dim", TermCell::TextModeEnum::Dim)
            .export_values();

    py::class_<TermUI, PyTermUI<>, std::shared_ptr<TermUI>> term_ui(m, "TermUI", plugin);
    term_ui.def(py::init<>())
            .def("start_main_ui_loop", &TermUI::StartMainUILoop)
            .def("create_window", &TermUI::CreateWindow)
            .def("schedule_task", &TermUI::ScheduleTask)
            ;

    py::class_<TermWindow, PyTermWindow<>, std::shared_ptr<TermWindow>> term_window(m, "TermWindow", plugin);
    term_window.def(py::init<>())
            .def("refresh", &TermWindow::Refresh)
            .def("show", &TermWindow::Show)
            .def("set_window_title", &TermWindow::SetWindowTitle)
            .def("get_color_by_index", &TermWindow::GetColorByIndex)
            .def_property("selection_data", &TermWindow::GetSelectionData, &TermWindow::SetSelectionData)
            .def("close", &TermWindow::Close)
            .def("enable_mouse_track", &TermWindow::EnableMouseTrack)
            ;

    py::class_<TermNetwork, PyTermNetwork<>, std::shared_ptr<TermNetwork>> term_network(m, "TermNetwork", multiple_instance_plugin);
    term_network.def(py::init<>())
            .def("disconnect", &TermNetwork::Disconnect)
            .def("connect", &TermNetwork::Connect)
            .def("send", &TermNetwork::Send)
            .def("resize", &TermNetwork::Resize)
            ;

    py::class_<TermContext, PyTermContext<>, std::shared_ptr<TermContext>> term_context(m, "TermContext", context);
    term_context.def(py::init<>())
            .def_property("term_buffer", &TermContext::GetTermBuffer, &TermContext::SetTermBuffer)
            .def_property("term_window", &TermContext::GetTermWindow, &TermContext::SetTermWindow)
            .def_property("term_network", &TermContext::GetTermNetwork, &TermContext::SetTermNetwork)
            .def_property("term_data_handler", &TermContext::GetTermDataHandler, &TermContext::SetTermDataHandler)
            .def_property("term_color_theme", &TermContext::GetTermColorTheme, &TermContext::SetTermColorTheme)
            .def_property("input_plugin", &TermContext::GetInputPlugin, &TermContext::SetInputPlugin)
            ;

    py::class_<PluginManager, PyPluginManager<>, std::shared_ptr<PluginManager>> plugin_manager(m, "PluginManager");
    plugin_manager.def(py::init<>())
            .def("register_plugin", (void(PluginManager::*)(std::shared_ptr<Plugin>))&PluginManager::RegisterPlugin)
            .def("register_plugin", (void(PluginManager::*)(const char*))&PluginManager::RegisterPlugin)
            .def("get_plugin", &PluginManager::GetPlugin);

    py::enum_<PluginManager::VersionCodeEnum>(plugin_manager, "VersionCode")
            .value("Latest", PluginManager::VersionCodeEnum::Latest)
            .export_values();

    py::class_<AppConfig, PyAppConfig<>, std::shared_ptr<AppConfig>> app_config(m, "AppConfig");
    app_config.def(py::init<>())
            .def("get_entry", &AppConfig::GetEntry)
            .def("get_entry_int64", &AppConfig::GetEntryInt64)
            .def("get_entry_uint64", &AppConfig::GetEntryUInt64)
            .def("get_entry_bool", &AppConfig::GetEntryBool)
            .def("load_from_file", &AppConfig::LoadFromFile)
            .def("load_from_string", &AppConfig::LoadFromString)
            ;
    py::class_<TermDataHandler, PyTermDataHandler<>, std::shared_ptr<TermDataHandler>> term_data_handler(m, "TermDataHandler", multiple_instance_plugin);
    term_data_handler.def(py::init<>())
            .def("on_data", &TermDataHandler::OnData)
            .def("start", &TermDataHandler::Start)
            .def("stop", &TermDataHandler::Stop)
            ;

    py::class_<Task, PyTask<>, std::shared_ptr<Task>> task(m, "Task", multiple_instance_plugin);
    task.def(py::init<>())
            .def("run", &Task::Run)
            .def("cancel", &Task::Cancel)
            .def("is_cancelled", &Task::IsCancelled)
            ;

    py::class_<TermColor, std::shared_ptr<TermColor>> term_color(m, "TermColor");
    term_color.def(py::init<>())
            .def_readwrite("r", &TermColor::r)
            .def_readwrite("g", &TermColor::g)
            .def_readwrite("b", &TermColor::b)
            .def_readwrite("a", &TermColor::a)
            ;

    py::class_<TermColorTheme, PyTermColorTheme<>, std::shared_ptr<TermColorTheme>> term_color_theme(m, "TermColorTheme", multiple_instance_plugin);
    term_color_theme.def(py::init<>())
            .def("load", &TermColorTheme::Load)
            .def("load_with_values", &TermColorTheme::LoadWithValues)
            .def("get_color", &TermColorTheme::GetColor)
            ;

    py::class_<InputPlugin, PyInputPlugin<>, std::shared_ptr<InputPlugin>> input_plugin(m, "InputPlugin", multiple_instance_plugin);
    input_plugin.def(py::init<>())
            .def("process_key", &InputPlugin::ProcessKey)
            .def("process_char_input", &InputPlugin::ProcessCharInput)
            .def("process_mouse_button", &InputPlugin::ProcessMouseButton)
            .def("process_mouse_move", &InputPlugin::ProcessMouseMove)
            ;
    py::enum_<InputPlugin::KeyCodeEnum>(input_plugin, "KeyCode")
            .value("KEY_UNKNOWN", InputPlugin::KeyCodeEnum::KEY_UNKNOWN)
            .value("KEY_SPACE", InputPlugin::KeyCodeEnum::KEY_SPACE)
            .value("KEY_APOSTROPHE", InputPlugin::KeyCodeEnum::KEY_APOSTROPHE)
            .value("KEY_COMMA", InputPlugin::KeyCodeEnum::KEY_COMMA)
            .value("KEY_MINUS", InputPlugin::KeyCodeEnum::KEY_MINUS)
            .value("KEY_PERIOD", InputPlugin::KeyCodeEnum::KEY_PERIOD)
            .value("KEY_SLASH", InputPlugin::KeyCodeEnum::KEY_SLASH)
            .value("KEY_0", InputPlugin::KeyCodeEnum::KEY_0)
            .value("KEY_1", InputPlugin::KeyCodeEnum::KEY_1)
            .value("KEY_2", InputPlugin::KeyCodeEnum::KEY_2)
            .value("KEY_3", InputPlugin::KeyCodeEnum::KEY_3)
            .value("KEY_4", InputPlugin::KeyCodeEnum::KEY_4)
            .value("KEY_5", InputPlugin::KeyCodeEnum::KEY_5)
            .value("KEY_6", InputPlugin::KeyCodeEnum::KEY_6)
            .value("KEY_7", InputPlugin::KeyCodeEnum::KEY_7)
            .value("KEY_8", InputPlugin::KeyCodeEnum::KEY_8)
            .value("KEY_9", InputPlugin::KeyCodeEnum::KEY_9)
            .value("KEY_SEMICOLON", InputPlugin::KeyCodeEnum::KEY_SEMICOLON)
            .value("KEY_EQUAL", InputPlugin::KeyCodeEnum::KEY_EQUAL)
            .value("KEY_A", InputPlugin::KeyCodeEnum::KEY_A)
            .value("KEY_B", InputPlugin::KeyCodeEnum::KEY_B)
            .value("KEY_C", InputPlugin::KeyCodeEnum::KEY_C)
            .value("KEY_D", InputPlugin::KeyCodeEnum::KEY_D)
            .value("KEY_E", InputPlugin::KeyCodeEnum::KEY_E)
            .value("KEY_F", InputPlugin::KeyCodeEnum::KEY_F)
            .value("KEY_G", InputPlugin::KeyCodeEnum::KEY_G)
            .value("KEY_H", InputPlugin::KeyCodeEnum::KEY_H)
            .value("KEY_I", InputPlugin::KeyCodeEnum::KEY_I)
            .value("KEY_J", InputPlugin::KeyCodeEnum::KEY_J)
            .value("KEY_K", InputPlugin::KeyCodeEnum::KEY_K)
            .value("KEY_L", InputPlugin::KeyCodeEnum::KEY_L)
            .value("KEY_M", InputPlugin::KeyCodeEnum::KEY_M)
            .value("KEY_N", InputPlugin::KeyCodeEnum::KEY_N)
            .value("KEY_O", InputPlugin::KeyCodeEnum::KEY_O)
            .value("KEY_P", InputPlugin::KeyCodeEnum::KEY_P)
            .value("KEY_Q", InputPlugin::KeyCodeEnum::KEY_Q)
            .value("KEY_R", InputPlugin::KeyCodeEnum::KEY_R)
            .value("KEY_S", InputPlugin::KeyCodeEnum::KEY_S)
            .value("KEY_T", InputPlugin::KeyCodeEnum::KEY_T)
            .value("KEY_U", InputPlugin::KeyCodeEnum::KEY_U)
            .value("KEY_V", InputPlugin::KeyCodeEnum::KEY_V)
            .value("KEY_W", InputPlugin::KeyCodeEnum::KEY_W)
            .value("KEY_X", InputPlugin::KeyCodeEnum::KEY_X)
            .value("KEY_Y", InputPlugin::KeyCodeEnum::KEY_Y)
            .value("KEY_Z", InputPlugin::KeyCodeEnum::KEY_Z)
            .value("KEY_LEFT_BRACKET", InputPlugin::KeyCodeEnum::KEY_LEFT_BRACKET)
            .value("KEY_BACKSLASH", InputPlugin::KeyCodeEnum::KEY_BACKSLASH)
            .value("KEY_RIGHT_BRACKET", InputPlugin::KeyCodeEnum::KEY_RIGHT_BRACKET)
            .value("KEY_GRAVE_ACCENT", InputPlugin::KeyCodeEnum::KEY_GRAVE_ACCENT)
            .value("KEY_WORLD_1", InputPlugin::KeyCodeEnum::KEY_WORLD_1)
            .value("KEY_WORLD_2", InputPlugin::KeyCodeEnum::KEY_WORLD_2)
            .value("KEY_ESCAPE", InputPlugin::KeyCodeEnum::KEY_ESCAPE)
            .value("KEY_ENTER", InputPlugin::KeyCodeEnum::KEY_ENTER)
            .value("KEY_TAB", InputPlugin::KeyCodeEnum::KEY_TAB)
            .value("KEY_BACKSPACE", InputPlugin::KeyCodeEnum::KEY_BACKSPACE)
            .value("KEY_INSERT", InputPlugin::KeyCodeEnum::KEY_INSERT)
            .value("KEY_DELETE", InputPlugin::KeyCodeEnum::KEY_DELETE)
            .value("KEY_RIGHT", InputPlugin::KeyCodeEnum::KEY_RIGHT)
            .value("KEY_LEFT", InputPlugin::KeyCodeEnum::KEY_LEFT)
            .value("KEY_DOWN", InputPlugin::KeyCodeEnum::KEY_DOWN)
            .value("KEY_UP", InputPlugin::KeyCodeEnum::KEY_UP)
            .value("KEY_PAGE_UP", InputPlugin::KeyCodeEnum::KEY_PAGE_UP)
            .value("KEY_PAGE_DOWN", InputPlugin::KeyCodeEnum::KEY_PAGE_DOWN)
            .value("KEY_HOME", InputPlugin::KeyCodeEnum::KEY_HOME)
            .value("KEY_END", InputPlugin::KeyCodeEnum::KEY_END)
            .value("KEY_CAPS_LOCK", InputPlugin::KeyCodeEnum::KEY_CAPS_LOCK)
            .value("KEY_SCROLL_LOCK", InputPlugin::KeyCodeEnum::KEY_SCROLL_LOCK)
            .value("KEY_NUM_LOCK", InputPlugin::KeyCodeEnum::KEY_NUM_LOCK)
            .value("KEY_PRINT_SCREEN", InputPlugin::KeyCodeEnum::KEY_PRINT_SCREEN)
            .value("KEY_PAUSE", InputPlugin::KeyCodeEnum::KEY_PAUSE)
            .value("KEY_F1", InputPlugin::KeyCodeEnum::KEY_F1)
            .value("KEY_F2", InputPlugin::KeyCodeEnum::KEY_F2)
            .value("KEY_F3", InputPlugin::KeyCodeEnum::KEY_F3)
            .value("KEY_F4", InputPlugin::KeyCodeEnum::KEY_F4)
            .value("KEY_F5", InputPlugin::KeyCodeEnum::KEY_F5)
            .value("KEY_F6", InputPlugin::KeyCodeEnum::KEY_F6)
            .value("KEY_F7", InputPlugin::KeyCodeEnum::KEY_F7)
            .value("KEY_F8", InputPlugin::KeyCodeEnum::KEY_F8)
            .value("KEY_F9", InputPlugin::KeyCodeEnum::KEY_F9)
            .value("KEY_F10", InputPlugin::KeyCodeEnum::KEY_F10)
            .value("KEY_F11", InputPlugin::KeyCodeEnum::KEY_F11)
            .value("KEY_F12", InputPlugin::KeyCodeEnum::KEY_F12)
            .value("KEY_F13", InputPlugin::KeyCodeEnum::KEY_F13)
            .value("KEY_F14", InputPlugin::KeyCodeEnum::KEY_F14)
            .value("KEY_F15", InputPlugin::KeyCodeEnum::KEY_F15)
            .value("KEY_F16", InputPlugin::KeyCodeEnum::KEY_F16)
            .value("KEY_F17", InputPlugin::KeyCodeEnum::KEY_F17)
            .value("KEY_F18", InputPlugin::KeyCodeEnum::KEY_F18)
            .value("KEY_F19", InputPlugin::KeyCodeEnum::KEY_F19)
            .value("KEY_F20", InputPlugin::KeyCodeEnum::KEY_F20)
            .value("KEY_F21", InputPlugin::KeyCodeEnum::KEY_F21)
            .value("KEY_F22", InputPlugin::KeyCodeEnum::KEY_F22)
            .value("KEY_F23", InputPlugin::KeyCodeEnum::KEY_F23)
            .value("KEY_F24", InputPlugin::KeyCodeEnum::KEY_F24)
            .value("KEY_F25", InputPlugin::KeyCodeEnum::KEY_F25)
            .value("KEY_KP_0", InputPlugin::KeyCodeEnum::KEY_KP_0)
            .value("KEY_KP_1", InputPlugin::KeyCodeEnum::KEY_KP_1)
            .value("KEY_KP_2", InputPlugin::KeyCodeEnum::KEY_KP_2)
            .value("KEY_KP_3", InputPlugin::KeyCodeEnum::KEY_KP_3)
            .value("KEY_KP_4", InputPlugin::KeyCodeEnum::KEY_KP_4)
            .value("KEY_KP_5", InputPlugin::KeyCodeEnum::KEY_KP_5)
            .value("KEY_KP_6", InputPlugin::KeyCodeEnum::KEY_KP_6)
            .value("KEY_KP_7", InputPlugin::KeyCodeEnum::KEY_KP_7)
            .value("KEY_KP_8", InputPlugin::KeyCodeEnum::KEY_KP_8)
            .value("KEY_KP_9", InputPlugin::KeyCodeEnum::KEY_KP_9)
            .value("KEY_KP_DECIMAL", InputPlugin::KeyCodeEnum::KEY_KP_DECIMAL)
            .value("KEY_KP_DIVIDE", InputPlugin::KeyCodeEnum::KEY_KP_DIVIDE)
            .value("KEY_KP_MULTIPLY", InputPlugin::KeyCodeEnum::KEY_KP_MULTIPLY)
            .value("KEY_KP_SUBTRACT", InputPlugin::KeyCodeEnum::KEY_KP_SUBTRACT)
            .value("KEY_KP_ADD", InputPlugin::KeyCodeEnum::KEY_KP_ADD)
            .value("KEY_KP_ENTER", InputPlugin::KeyCodeEnum::KEY_KP_ENTER)
            .value("KEY_KP_EQUAL", InputPlugin::KeyCodeEnum::KEY_KP_EQUAL)
            .value("KEY_LEFT_SHIFT", InputPlugin::KeyCodeEnum::KEY_LEFT_SHIFT)
            .value("KEY_LEFT_CONTROL", InputPlugin::KeyCodeEnum::KEY_LEFT_CONTROL)
            .value("KEY_LEFT_ALT", InputPlugin::KeyCodeEnum::KEY_LEFT_ALT)
            .value("KEY_LEFT_SUPER", InputPlugin::KeyCodeEnum::KEY_LEFT_SUPER)
            .value("KEY_RIGHT_SHIFT", InputPlugin::KeyCodeEnum::KEY_RIGHT_SHIFT)
            .value("KEY_RIGHT_CONTROL", InputPlugin::KeyCodeEnum::KEY_RIGHT_CONTROL)
            .value("KEY_RIGHT_ALT", InputPlugin::KeyCodeEnum::KEY_RIGHT_ALT)
            .value("KEY_RIGHT_SUPER", InputPlugin::KeyCodeEnum::KEY_RIGHT_SUPER)
            .value("KEY_MENU", InputPlugin::KeyCodeEnum::KEY_MENU)
            .value("KEY_LAST", InputPlugin::KeyCodeEnum::KEY_LAST)
            .export_values();
    py::enum_<InputPlugin::ModifierEnum>(input_plugin, "Modifier")
            .value("M_SHIFT", InputPlugin::ModifierEnum::M_SHIFT)
            .value("M_CONTROL", InputPlugin::ModifierEnum::M_CONTROL)
            .value("M_ALT", InputPlugin::ModifierEnum::M_ALT)
            .value("M_SUPER", InputPlugin::ModifierEnum::M_SUPER)
            .export_values();
    py::enum_<InputPlugin::MouseButtonEnum>(input_plugin, "MouseButton")
            .value("MOUSE_BUTTON_1", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_1)
            .value("MOUSE_BUTTON_2", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_2)
            .value("MOUSE_BUTTON_3", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_3)
            .value("MOUSE_BUTTON_4", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_4)
            .value("MOUSE_BUTTON_5", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_5)
            .value("MOUSE_BUTTON_6", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_6)
            .value("MOUSE_BUTTON_7", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_7)
            .value("MOUSE_BUTTON_8", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_8)
            .value("MOUSE_BUTTON_LAST", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_LAST)
            .value("MOUSE_BUTTON_LEFT", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_LEFT)
            .value("MOUSE_BUTTON_RIGHT", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_RIGHT)
            .value("MOUSE_BUTTON_MIDDLE", InputPlugin::MouseButtonEnum::MOUSE_BUTTON_MIDDLE)
            .export_values();
}

void init_wxglterm_interface_module()
{
    auto py_module1 = py::module::import("wxglterm_interface");

    py::print(py_module1);
}
