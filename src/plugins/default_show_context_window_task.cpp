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
#include "plugin_base.h"

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
