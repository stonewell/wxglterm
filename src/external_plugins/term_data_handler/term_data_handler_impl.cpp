#include <pybind11/embed.h>

#include <iostream>
#include <vector>
#include <list>
#include <chrono>

#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>
#endif

#include "plugin.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"
#include "term_buffer.h"
#include "term_window.h"

#include "PortableThread.h"

#include "term_data_handler_impl.h"
#include "term_data_handler_impl_decl.h"

#include "load_module.h"

namespace py = pybind11;

class __SimpleGILStateLock {
public:
    __SimpleGILStateLock() {
        gstate = PyGILState_Ensure();
    }
    ~__SimpleGILStateLock() {
        PyGILState_Release(gstate);
    }

    PyGILState_STATE gstate;
};

#if !defined(_WIN32) || defined(__GNUC__)
#pragma GCC visibility push(hidden)
#endif

TermDataHandlerPtr CreateTermDataHandler()
{
    return TermDataHandlerPtr { new TermDataHandlerImpl };
}

void TermDataHandlerImpl::ProcessSingleChar(const char * ch) {
    if (m_UsePythonImpl) {
        ProcessSingleCharPy(ch);
    } else {
        ProcessSingleCharNative(ch);
    }
}

void TermDataHandlerImpl::ProcessSingleCharPy(const char * ch) {
    std::vector<term_data_param_s> int_params;
    std::string str_cap_name {};
    bool has_cap = false;
    bool has_output_char = false;
    char c = 0;

    {
        __SimpleGILStateLock lock;

        if (ch)
            m_DataHandler.attr("on_term_data")(*ch, m_DataContext.cap_debug);
        else
            m_DataHandler.attr("on_term_data")(py::arg("cap_debug") = m_DataContext.cap_debug);

        py::object cap_name = m_DataHandler.attr("cap_name");
        py::object params = m_DataHandler.attr("params");
        py::object output_data = m_DataHandler.attr("output_char");
        py::object increase_param = m_DataHandler.attr("increase_param");

        if (!cap_name.is_none()) {
            str_cap_name = cap_name.cast<std::string>();
            bool b_increase_param = increase_param.cast<bool>();
            has_cap = true;

            if (!params.is_none()) {
                py::list l = params;
                for(auto i : l) {
                    term_data_param_s param {};

                    try {
                        int ii = i.cast<int>();
                        if (b_increase_param && ii > 0)
                            ii--;

                        param.int_value = ii;
                        param.has_int_value = true;
                    }catch(...) {
                        param.str_value = i.cast<std::string>();
                        param.has_str_value = true;
                    }

                    int_params.push_back(param);
                }
            }
        }

        if (!output_data.is_none()) {
            c = output_data.cast<char>() & 0xFF;
            has_output_char = true;
        }
    }

    if (has_cap) {
        handle_cap(m_DataContext,
                   str_cap_name,
                   int_params);
    }

    if (has_output_char) {
        output_char(m_DataContext, c, false);
    }
}

void TermDataHandlerImpl::ProcessAllChars(char ch) {
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    do {
        ProcessSingleChar(&ch);

        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() >= 50 * 1000) {
            start = end;

            m_DataContext.term_window->Refresh();
        }
    } while (!m_Stopped && m_TermDataQueue.try_dequeue(ch));

    if (!m_Stopped)
        ProcessSingleChar(NULL);
}

unsigned long TermDataHandlerImpl::Run(void * /*pArgument*/) {

    while(!m_Stopped) {
        char c = '\0';

        if (!m_TermDataQueue.wait_dequeue_timed(c, std::chrono::microseconds(1000000))) {
            if (m_DataContext.cap_debug) {
                std::cout << "queue wait timed out ..." << std::endl;
            }

            if (m_Stopped)
                break;

            continue;
        }

        if (m_Stopped)
            break;

        //process char
        ProcessAllChars(c);

        m_DataContext.term_window->Refresh();
    };

    return 0;
}

void TermDataHandlerImpl::OnData(const std::vector<unsigned char> & data, size_t data_len) {
    for (size_t i = 0; i < data_len; i++) {
        m_TermDataQueue.enqueue(data[i]);
    }
}

void TermDataHandlerImpl::LoadPyDataHandler() {
    try
    {
        const char *module_content =
#include "term_data_handler_impl.inc"
                ;

        std::string termcap_dir = GetPluginContext()->GetAppConfig()->GetEntry("/term/termcap_dir", "data");
        std::string term_name = GetPluginContext()->GetAppConfig()->GetEntry("/term/term_name", "xterm-256color");
        bool use_generic = GetPluginContext()->GetAppConfig()->GetEntryBool("/term/use_generic_cap", "true");

        m_DataHandler =
                LoadPyModuleFromString(module_content,
                                       "term_data_handler_impl",
                                       "term_data_handler_impl.py").attr("create_data_handler")(termcap_dir, term_name, use_generic);
    }
    catch(std::exception & e)
    {
        std::cerr << "load python data handler failed!"
                  << std::endl
                  << e.what()
                  << std::endl;
    }
    catch(std::string & e) {
        std::cerr << "load python data handler failed!"
                  << std::endl
                  << e
                  << std::endl;
    }
    catch(...)
    {
        std::cerr << "load python data handler failed!"
                  << std::endl;
        PyErr_Print();
    }
}

void TermDataHandlerImpl::Start() {
    if (!m_Stopped)
        return;

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    m_DataContext.term_buffer = context->GetTermBuffer();
    m_DataContext.term_window = context->GetTermWindow();
    m_DataContext.term_network = context->GetTermNetwork();
    m_DataContext.cell_template = m_DataContext.term_buffer->CreateCellWithDefaults();
    m_DataContext.default_cell_template = m_DataContext.term_buffer->CreateCellWithDefaults();

    m_Stopped = false;

    m_DataHandlerThread.Start();
}

void TermDataHandlerImpl::Stop() {
    if (m_Stopped)
        return;

    m_Stopped = true;

    std::vector<unsigned char> data;
    data.push_back('A');

    OnData(data, 1);

    m_DataHandlerThread.Join();

    m_DataContext.term_buffer = nullptr;
    m_DataContext.term_window = nullptr;
    m_DataContext.term_network = nullptr;
    m_DataContext.cell_template = nullptr;
    m_DataContext.default_cell_template = nullptr;
}

#if !defined(_WIN32) || defined(__GNUC__)
#pragma GCC visibility pop
#endif
