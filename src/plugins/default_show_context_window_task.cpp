#include <pybind11/embed.h>

#include "plugin.h"
#include "plugin_manager.h"
#include "default_show_context_window_task.h"
#include "term_window.h"
#include "task.h"
#include "term_context.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "app_config_impl.h"

class PluginBase : public virtual Plugin {
public:
    PluginBase(const char * name, const char * description, uint32_t version);
    virtual ~PluginBase() = default;

public:
    const char * GetName() override {
        return m_Name.c_str();
    }
    const char * GetDescription() override {
        return m_Description.c_str();
    }

    uint32_t GetVersion() override {
        return m_Version;
    }

    virtual void InitPlugin(ContextPtr context,
                    AppConfigPtr plugin_config) override {
        m_Context = context;
        m_PluginConfig = plugin_config;
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
};

PluginBase::PluginBase(const char * name, const char * description, uint32_t version) :
    m_Name(name)
    , m_Description(description)
    , m_Version(version)
    , m_Context{}
    , m_PluginConfig{}
{
}

class __ShowContextWindowTask : public virtual PluginBase, public virtual Task {
public:
    __ShowContextWindowTask() :
        PluginBase("show_context_window_task", "default task show main window", 1)
        , m_Cancelled(false)
    {
    }

    virtual ~__ShowContextWindowTask() = default;

public:
    void Run() override {
        if (m_Cancelled)
            return;

        pybind11::gil_scoped_acquire acquire;
        auto mainWnd = std::dynamic_pointer_cast<TermContext>(m_Context)->GetTermWindow();
        mainWnd->Show();

        auto term_network = std::dynamic_pointer_cast<TermContext>(m_Context)->GetTermNetwork();
        term_network->Connect("", 0, "", "");

        auto term_dataHandler = std::dynamic_pointer_cast<TermContext>(m_Context)->GetTermDataHandler();
        term_dataHandler->Start();
    }

    void Cancel() override {
        m_Cancelled = true;
    }

    bool IsCancelled() override {
        return m_Cancelled;
    }

    MultipleInstancePluginPtr NewInstance() override{
        return MultipleInstancePluginPtr { new __ShowContextWindowTask() };
    }

private:
    bool m_Cancelled;
};


TaskPtr CreateDefaultShowContextWindowTask() {
    return TaskPtr { new __ShowContextWindowTask() };
}
