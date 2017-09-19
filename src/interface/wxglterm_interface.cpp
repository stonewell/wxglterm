#include <iostream>

#include <pybind11/pybind11.h>

#include "py_term_buffer.h"
#include "py_term_ui.h"
#include "py_term_network.h"
#include "py_term_context.h"

namespace py = pybind11;

void print_plugin_info(Plugin * plugin)
{
    std::cerr << "Name:" << plugin->GetName()
              << ", Description:" << plugin->GetDescription()
              << ", Version:" << plugin->GetVersion()
              << std::endl;
}

PYBIND11_MODULE(wxglterm_interface, m)
{
    m.def("print_plugin_info", &print_plugin_info);

    py::class_<Plugin, PyPlugin<>> plugin(m, "Plugin");
    plugin.def(py::init<>())
            .def("get_name", &Plugin::GetName)
            .def("get_description", &Plugin::GetDescription)
            .def("get_version", &Plugin::GetVersion);

    py::class_<Context, PyContext<>> context(m, "Context");
    context.def(py::init<>());

    py::class_<TermBuffer, PyTermBuffer<>> term_buffer(m, "TermBuffer", plugin);
    term_buffer.def(py::init<>());

    py::class_<TermUI, PyTermUI<>> term_ui(m, "TermUI", plugin);
    term_ui.def(py::init<>())
            .def("refresh", &TermUI::Refresh);

    py::class_<TermNetwork, PyTermNetwork<>> term_network(m, "TermNetwork", plugin);
    term_network.def(py::init<>())
            .def("disconnect", &TermNetwork::Disconnect);

    py::class_<TermContext, PyTermContext<>> term_context(m, "TermContext", context);
    term_context.def(py::init<>())
            .def("get_term_buffer", &TermContext::GetTermBuffer)
            .def("get_term_ui", &TermContext::GetTermUI)
            .def("get_term_network", &TermContext::GetTermNetwork);
}
