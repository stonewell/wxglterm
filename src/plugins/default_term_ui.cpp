#include "plugin_base.h"

#include "default_term_ui.h"
#include "term_window.h"
#include "task.h"
#include "term_context.h"
#include "term_network.h"

#include "main_dlg.h"

class __wxGLTermApp : public wxApp {
public:
    virtual bool OnInit() {
        return true;
    }
};

class __wxGLTimer : public wxTimer {
public:
    __wxGLTimer(TaskPtr task, int miliseconds):
        m_Task(task)
        , m_Interval(miliseconds)
    {
        (void)m_Interval;
    }

    virtual ~__wxGLTimer() = default;

public:
    virtual void Notify() {
        if (m_Task && !m_Task->IsCancelled()) {
            m_Task->Run();
        }
    }

private:
    TaskPtr m_Task;
    int m_Interval;
};

wxIMPLEMENT_APP_NO_MAIN(__wxGLTermApp);

class DefaultTermWindow : public virtual PluginBase, public virtual TermWindow, public WindowManager {
public:
    DefaultTermWindow() :
        PluginBase("default_term_window", "default terminal window plugin", 0)
        , m_MainDlg(nullptr) {
    }

    virtual ~DefaultTermWindow() = default;

public:
    void Refresh() override {
        wxCriticalSectionLocker locker(m_MainWndLock);

        if (!m_MainDlg)
            return;

        m_MainDlg->RequestRefresh();
    }

    void WindowClosed(wxFrame * win) {
        wxCriticalSectionLocker locker(m_MainWndLock);
        if (win == m_MainDlg)
            m_MainDlg = nullptr;
    }

    void Show() override {
        if (!m_MainDlg)
            m_MainDlg = new MainDialog(wxT("wxGLTerm"), this);

        m_MainDlg->Show(true);
    }

private:
    MainDialog * m_MainDlg;
    wxCriticalSection m_MainWndLock;
};

class DefaultTermUI : public virtual PluginBase, public virtual TermUI {
public:
    DefaultTermUI() :
        PluginBase("default_term_ui", "default terminal ui plugin", 0)
    {
    }

    virtual ~DefaultTermUI() = default;

    TermWindowPtr CreateWindow() {
        return TermWindowPtr { new DefaultTermWindow() };
    }

    int32_t StartMainUILoop() {
        int argc = 0;

        return wxEntry(argc, (char **)nullptr);
    }

    bool ScheduleTask(TaskPtr task, int miliseconds, bool repeated) {
        auto timer = new __wxGLTimer(task, miliseconds);
        return timer->Start(miliseconds, !repeated);
    }
};

class __ShowContextWindowTask : public virtual PluginBase, public virtual Task {
public:
    __ShowContextWindowTask() :
        PluginBase("show_context_window_task", "default task show main window", 0)
        , m_Cancelled(false)
    {
    }

    virtual ~__ShowContextWindowTask() = default;

public:
    void Run() override {
        if (m_Cancelled)
            return;

        auto mainWnd = std::dynamic_pointer_cast<TermContext>(m_Context)->GetTermWindow();
        mainWnd->Show();
        auto term_network = std::dynamic_pointer_cast<TermContext>(m_Context)->GetTermNetwork();

        term_network->Connect("", 0, "", "");
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

TermUIPtr CreateDefaultTermUI()
{
    if (!wxApp::GetInstance())
        wxCreateApp();
    return TermUIPtr{ new DefaultTermUI()};
}

TaskPtr CreateShowContextWindowTask() {
    return TaskPtr { new __ShowContextWindowTask() };
}
