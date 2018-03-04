#include <pybind11/embed.h>

#include <iostream>
#include <vector>
#include <list>
#include <chrono>

#include <unistd.h>
#include <string.h>

#include <sys/select.h>
#include <sys/wait.h>

#include "plugin.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"
#include "term_buffer.h"
#include "term_window.h"

#include "PortableThread.h"

#include "scintilla_editor_impl.h"
#include "scintilla_editor_impl_decl.h"

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

#pragma GCC visibility push(hidden)

TermDataHandlerPtr CreateTermDataHandler()
{
    return TermDataHandlerPtr { new ScintillaEditorDataHandlerImpl };
}

void ScintillaEditorDataHandlerImpl::ProcessSingleChar(const char * ch) {
    ProcessSingleCharNative(ch);
}

void ScintillaEditorDataHandlerImpl::ProcessAllChars(char ch) {
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

unsigned long ScintillaEditorDataHandlerImpl::Run(void * /*pArgument*/) {

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

void ScintillaEditorDataHandlerImpl::OnData(const std::vector<unsigned char> & data, size_t data_len) {
    for (size_t i = 0; i < data_len; i++) {
        m_TermDataQueue.enqueue(data[i]);
    }
}

void ScintillaEditorDataHandlerImpl::Start() {
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

void ScintillaEditorDataHandlerImpl::Stop() {
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
#pragma GCC visibility pop
