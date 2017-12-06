#pragma once

#include <pybind11/embed.h>
#include <deque>

#include "cap_manager.h"
#include "read_write_queue.h"
#include "parse_termdata.h"

using TermDataQueue = moodycamel::BlockingReaderWriterQueue<unsigned char, 4096>;

class __attribute__ ((visibility ("hidden"))) TermDataHandlerImpl
        : public virtual Plugin
        , public virtual TermDataHandler
        , public virtual PortableThread::IPortableRunnable
{
public:
    TermDataHandlerImpl() :
        Plugin()
        , TermDataHandler()
        , PortableThread::IPortableRunnable()
        , m_Name("term_data_handler")
        , m_Description("terminal data handler")
        , m_Version(1)
        , m_DataHandlerThread(this)
        , m_TermDataQueue {4096}
        , m_Stopped{true}
        , m_UsePythonImpl{true}
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
