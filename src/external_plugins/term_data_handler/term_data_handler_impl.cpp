#include <pybind11/embed.h>

#include <iostream>
#include <vector>
#include <list>

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

#include "term_data_handler_impl.h"
#include "read_write_queue.h"
#include "load_module.h"

#include "cap_manager.h"

using TermDataQueue = moodycamel::BlockingReaderWriterQueue<unsigned char, 4096>;

namespace py = pybind11;

#pragma GCC visibility push(hidden)
class TermDataHandlerImpl
        : public virtual Plugin
        , public virtual TermDataHandler
        , public virtual PortableThread::IPortableRunnable
{
public:
    TermDataHandlerImpl() :
        Plugin()
        , m_Name("term_data_handler")
        , m_Description("terminal data handler")
        , m_Version(1)
        , m_DataHandlerThread(this)
        , m_TermDataQueue {4096}
        , m_Stopped{true}
    {
    }

    virtual ~TermDataHandlerImpl() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new TermDataHandlerImpl};
    }

    void InitPlugin(ContextPtr context,
                    AppConfigPtr plugin_config) override {
        m_Context = context;
        m_PluginConfig = plugin_config;

        bool app_debug = context->GetAppConfig()->GetEntryBool("app_debug", false);

        m_DataContext.cap_debug = plugin_config->GetEntryBool("cap_debug", app_debug);

        LoadPyDataHandler();
    }

    unsigned long Run(void * /*pArgument*/) override;
    void OnData(const std::vector<unsigned char> & data, size_t data_len) override;
    void Start() override;
    void Stop() override;

    const char * GetName() override {
        return m_Name.c_str();
    }
    const char * GetDescription() override {
        return m_Description.c_str();
    }

    uint32_t GetVersion() override {
        return m_Version;
    }

    ContextPtr GetPluginContext() const override {
        return m_Context;
    }
    AppConfigPtr GetPluginConfig() const override {
        return m_PluginConfig;
    }
private:
    std::string m_Name;
    std::string m_Description;
    uint32_t m_Version;

protected:
    ContextPtr m_Context;
    AppConfigPtr m_PluginConfig;

private:
    void LoadPyDataHandler();
    void ProcessSingleChar(const char * ch);
    void ProcessAllChars(char ch);

    PortableThread::CPortableThread m_DataHandlerThread;
    TermDataQueue m_TermDataQueue;
    bool m_Stopped;
    py::object m_DataHandler;

    term_data_context_s m_DataContext;
};

TermDataHandlerPtr CreateTermDataHandler()
{
    return TermDataHandlerPtr { new TermDataHandlerImpl };
}

void TermDataHandlerImpl::ProcessSingleChar(const char * ch) {
    if (ch)
        m_DataHandler.attr("on_term_data")(*ch, m_DataContext.cap_debug);
    else
        m_DataHandler.attr("on_term_data")(py::arg("cap_debug") = m_DataContext.cap_debug);

    py::object cap_name = m_DataHandler.attr("cap_name");
    py::object params = m_DataHandler.attr("params");
    py::object output_data = m_DataHandler.attr("output_char");
    py::object increase_param = m_DataHandler.attr("increase_param");

    if (!cap_name.is_none()) {
        std::string str_cap_name = cap_name.cast<std::string>();
        bool b_increase_param = increase_param.cast<bool>();

        std::vector<int> int_params;

        if (!params.is_none()) {
            py::list l = params;
            for(auto i : l) {
                int ii = i.cast<int>();

                if (b_increase_param && ii > 0)
                    ii--;
                int_params.push_back(ii);
            }
        }

        handle_cap(m_DataContext,
                   str_cap_name,
                   int_params);
    }
    if (!output_data.is_none()) {
        char c = output_data.cast<char>() & 0xFF;

        output_char(m_DataContext, c, false);
    }
}

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

void TermDataHandlerImpl::ProcessAllChars(char ch) {
    __SimpleGILStateLock lock;

    do {
        ProcessSingleChar(&ch);
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

        m_DataHandler =
                LoadPyModuleFromString(module_content,
                                       "term_data_handler_impl",
                                       "term_data_handler_impl.py").attr("create_data_handler")(termcap_dir, term_name);
    }
    catch(std::exception & e)
    {
        std::cerr << "load python data handler failed!"
                  << std::endl
                  << e.what()
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
#pragma GCC visibility pop
