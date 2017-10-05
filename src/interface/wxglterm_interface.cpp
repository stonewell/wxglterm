#include <iostream>

#ifdef WXGLTERM_DYNAMIC_INTERFACE
#include <pybind11/pybind11.h>
#else
#include <pybind11/embed.h>
#endif

#include "py_term_cell.h"
#include "py_term_line.h"
#include "py_term_buffer.h"
#include "py_term_ui.h"
#include "py_term_network.h"
#include "py_term_context.h"

#include "py_plugin_manager.h"

#include "py_multiple_instance_plugin.h"

#include "py_app_config.h"

#include "wxglterm_interface.h"

namespace py = pybind11;

void print_plugin_info(Plugin * plugin)
{
    TermNetwork * term_network = dynamic_cast<TermNetwork *>(plugin);

    std::cerr << "Name:" << plugin->GetName()
              << ", Description:" << plugin->GetDescription()
              << ", Version:" << plugin->GetVersion()
              << std::endl;

    if (term_network)
    {
        term_network->Disconnect();
    }
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
            .def("get_version", &Plugin::GetVersion);

    py::class_<MultipleInstancePlugin, PyMultipleInstancePlugin<>, std::shared_ptr<MultipleInstancePlugin>> multiple_instance_plugin(m, "MultipleInstancePlugin", plugin);
    multiple_instance_plugin.def(py::init<>())
            .def("new_instance", &MultipleInstancePlugin::NewInstance);

    py::class_<Context, PyContext<>, std::shared_ptr<Context>> context(m, "Context");
    context.def(py::init<>());

    py::class_<TermBuffer, PyTermBuffer<>, std::shared_ptr<TermBuffer>> term_buffer(m, "TermBuffer", multiple_instance_plugin);
    term_buffer.def(py::init<>())
            .def("resize", &TermBuffer::Resize)
            .def("get_rows", &TermBuffer::GetRows)
            .def("get_cols", &TermBuffer::GetCols)
            .def("get_row", &TermBuffer::GetRow)
            .def("get_col", &TermBuffer::GetCol)
            .def("get_line", &TermBuffer::GetLine)
            .def("get_cell", &TermBuffer::GetCell)
            .def("get_cur_line", &TermBuffer::GetCurLine)
            .def("get_cur_cell", &TermBuffer::GetCurCell)
            .def("scroll_buffer", &TermBuffer::ScrollBuffer)
            .def_property("scroll_region_begin", &TermBuffer::GetScrollRegionBegin, &TermBuffer::SetScrollRegionBegin)
            .def_property("scroll_region_end", &TermBuffer::GetScrollRegionEnd, &TermBuffer::SetScrollRegionEnd)
            ;

    py::class_<TermLine, PyTermLine<>, std::shared_ptr<TermLine>> term_line(m, "TermLine", plugin);
    term_line.def(py::init<>())
            .def("resize", &TermLine::Resize)
            .def("get_cell", &TermLine::GetCell)
            ;

    py::class_<TermCell, PyTermCell<>, std::shared_ptr<TermCell>> term_cell(m, "TermCell", plugin);
    term_cell.def(py::init<>())
            .def_property("char", &TermCell::GetChar, &TermCell::SetChar)
            .def_property("fore_color_idx", &TermCell::GetForeColorIndex, &TermCell::SetForeColorIndex)
            .def_property("back_color_idx", &TermCell::GetBackColorIndex, &TermCell::SetBackColorIndex)
            .def_property("mode", &TermCell::GetMode, &TermCell::SetMode)
            ;

    py::enum_<TermCell::ColorIndexEnum>(term_cell, "ColorIndex")
            .value("DefaultColorIndex", TermCell::ColorIndexEnum::DefaultColorIndex)
            .export_values();

    py::class_<TermUI, PyTermUI<>, std::shared_ptr<TermUI>> term_ui(m, "TermUI", multiple_instance_plugin);
    term_ui.def(py::init<>())
            .def("refresh", &TermUI::Refresh)
            .def("show", &TermUI::Show)
            .def("start_main_ui_loop", &TermUI::StartMainUILoop)
            ;

    py::class_<TermNetwork, PyTermNetwork<>, std::shared_ptr<TermNetwork>> term_network(m, "TermNetwork", plugin);
    term_network.def(py::init<>())
            .def("disconnect", &TermNetwork::Disconnect);

    py::class_<TermContext, PyTermContext<>, std::shared_ptr<TermContext>> term_context(m, "TermContext", context);
    term_context.def(py::init<>())
            .def("get_term_buffer", &TermContext::GetTermBuffer)
            .def("get_term_ui", &TermContext::GetTermUI)
            .def("get_term_network", &TermContext::GetTermNetwork)
            .def("set_term_buffer", &TermContext::SetTermBuffer)
            .def("set_term_ui", &TermContext::SetTermUI)
            .def("set_term_network", &TermContext::SetTermNetwork);

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
            .def("load_from_file", &AppConfig::LoadFromFile);
}

void init_wxglterm_interface_module()
{
    auto py_module1 = py::module::import("wxglterm_interface");

    py::print(py_module1);
}
