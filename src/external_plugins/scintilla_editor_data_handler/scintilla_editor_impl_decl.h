#pragma once

#include <pybind11/embed.h>
#include <deque>

#include "cap_manager.h"
#include "read_write_queue.h"
#include "parse_termdata.h"
#include "plugin_base.h"

using TermDataQueue = moodycamel::BlockingReaderWriterQueue<unsigned char, 4096>;

class __attribute__ ((visibility ("hidden"))) ScintillaEditorDataHandlerImpl
        : public virtual PluginBase
        , public virtual TermDataHandler
        , public virtual PortableThread::IPortableRunnable
{
public:
    ScintillaEditorDataHandlerImpl()
        : PluginBase("scintilla_editor_data_handler", "scintilla editor data handler", 1)
        , TermDataHandler()
        , PortableThread::IPortableRunnable()
        , m_DataHandlerThread(this)
        , m_TermDataQueue {4096}
        , m_Stopped{true}
    {
    }

    virtual ~ScintillaEditorDataHandlerImpl() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new ScintillaEditorDataHandlerImpl};
    }

    void InitPlugin(ContextPtr context,
                    AppConfigPtr plugin_config) override {
        PluginBase::InitPlugin(context, plugin_config);

        m_DataContext.cap_debug = m_Debug;

        LoadNativeDataHandler();
    }

    unsigned long Run(void * /*pArgument*/) override;
    void OnData(const std::vector<unsigned char> & data, size_t data_len) override;
    void Start() override;
    void Stop() override;

private:
    void LoadNativeDataHandler();
    void ProcessSingleChar(const char * ch);
    void ProcessAllChars(char ch);
    void ProcessSingleCharNative(const char * ch);
    void HandleCap(bool check_unknown = true, char c = 0);

    PortableThread::CPortableThread m_DataHandlerThread;
    TermDataQueue m_TermDataQueue;
    bool m_Stopped;
    pybind11::object m_DataHandler;

    term_data_context_s m_DataContext;

    CapPtr m_Cap;
    ControlDataParserContextPtr m_ParseContext;
    ControlDataStatePtr m_State;
    std::vector<char> m_ControlData;
    std::deque<std::tuple<ControlDataStatePtr, term_data_param_vector_t, std::vector<char>>> m_CapStateStack;
};
