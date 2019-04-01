#pragma once

#include <pybind11/embed.h>
#include <deque>

#include "cap_manager.h"
#include "read_write_queue.h"
#include "parse_termdata.h"
#include "plugin_base.h"

using TermDataQueue = moodycamel::BlockingReaderWriterQueue<unsigned char, 4096>;

#if !defined(_WIN32) || defined(__GNUC__)
#define ATTR_HIDDEN __attribute__ ((visibility ("hidden")))
#else
#define ATTR_HIDDEN
#endif

class ATTR_HIDDEN TermDataHandlerImpl
        : public TermDataHandler
        , public PortableThread::IPortableRunnable
{
public:
    TermDataHandlerImpl()
        : TermDataHandler()
        , PortableThread::IPortableRunnable()
        , PLUGIN_BASE_INIT_LIST("term_data_handler", "terminal data handler", 1)
        , m_DataHandlerThread(this)
        , m_TermDataQueue {4096}
        , m_Stopped{true}
        , m_UsePythonImpl{true}
    {
    }

    virtual ~TermDataHandlerImpl() = default;

	PLUGIN_BASE_DEFINE_PREFIX(, , , Internal_, , );

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new TermDataHandlerImpl};
    }

    void InitPlugin(ContextPtr context,
                    AppConfigPtr plugin_config) override {
        Internal_InitPlugin(context, plugin_config);

        m_DataContext.cap_debug = m_Debug;

        m_UsePythonImpl = plugin_config->GetEntryBool("use_python_impl",
                                                      false);

        if (m_UsePythonImpl) {
            LoadPyDataHandler();
        } else {
            LoadNativeDataHandler();
        }
    }

    unsigned long Run(void * /*pArgument*/) override;
    void OnData(const std::vector<unsigned char> & data, size_t data_len) override;
    void Start() override;
    void Stop() override;

private:
    void LoadPyDataHandler();
    void LoadNativeDataHandler();
    void ProcessSingleChar(const char * ch);
    void ProcessAllChars(char ch);
    void ProcessSingleCharPy(const char * ch);
    void ProcessSingleCharNative(const char * ch);
    void HandleCap(bool check_unknown = true, char c = 0);

    PortableThread::CPortableThread m_DataHandlerThread;
    TermDataQueue m_TermDataQueue;
    bool m_Stopped;
    pybind11::object m_DataHandler;

    term_data_context_s m_DataContext;

    bool m_UsePythonImpl;

    CapPtr m_Cap;
    ControlDataParserContextPtr m_ParseContext;
    ControlDataStatePtr m_State;
    std::vector<char> m_ControlData;
    std::deque<std::tuple<ControlDataStatePtr, term_data_param_vector_t, std::vector<char>>> m_CapStateStack;
};
