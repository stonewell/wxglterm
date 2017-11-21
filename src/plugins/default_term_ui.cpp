#include <pybind11/embed.h>

#include "plugin_base.h"
#include "default_term_ui.h"
#include "term_window.h"
#include "task.h"
#include "term_context.h"
#include "term_network.h"
#include "term_data_handler.h"

#include "main_dlg.h"

#include <wx/clipbrd.h>
#include <wx/base64.h>

class __wxGLTermApp : public wxApp {
public:
    virtual bool OnInit() {
        return true;
    }
};

class __wxGLTimer : public wxTimer {
public:
    __wxGLTimer(TaskPtr task, int miliseconds, bool repeated):
        m_Task(task)
        , m_Interval(miliseconds)
        , m_Repeated(repeated)
    {
        (void)m_Interval;
        (void)m_Repeated;
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
    bool m_Repeated;
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
            m_MainDlg = new MainDialog(wxT("wxGLTerm"), this, this);

        m_MainDlg->Show(true);
        m_MainDlg->Maximize(true);
    }

    void SetWindowTitle(const std::string & title) override {
        m_MainDlg->SetTitle(wxString(title.c_str()));
    }

    uint32_t GetColorByIndex(uint32_t index) override {
        return m_MainDlg->GetColorByIndex(index);
    }

    std::string GetSelectionData() override {
        std::string sel_data {};

        if (wxTheClipboard->Open()) {
            if (wxTheClipboard->IsSupported( wxDF_TEXT ) ||
                wxTheClipboard->IsSupported( wxDF_UNICODETEXT) ||
                wxTheClipboard->IsSupported( wxDF_OEMTEXT)) {
                wxTextDataObject data;
                wxTheClipboard->GetData( data );

                wxString s = data.GetText();
                const auto & s_buf = s.utf8_str();
                const char * s_begin = s_buf;

                sel_data = wxBase64Encode(s_begin,
                                          s_buf.length()).utf8_str();
            }
            wxTheClipboard->Close();
        }

        return sel_data;
    }

    void SetSelectionData(const std::string & sel_data) override {
        if (sel_data.size() == 0) {
            m_MainDlg->SetSelectionData("");
        } else {
            size_t pos_err = (size_t)-1;

            auto buf = wxBase64Decode(sel_data.c_str(),
                                      wxBase64DecodeMode_Strict,
                                      &pos_err);

            if (pos_err != (size_t)-1) {
                m_MainDlg->SetSelectionData("");
            } else {
                auto str_data = wxString::FromUTF8((char*)buf.GetData(), buf.GetDataLen());

                std::cerr << "set selection data:" << str_data.utf8_str() << std::endl;

                m_MainDlg->SetSelectionData(str_data);
            }
        }
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
        auto window = TermWindowPtr { new DefaultTermWindow() };
        window->InitPlugin(GetPluginContext(),
                           GetPluginConfig());

        return window;
    }

    int32_t StartMainUILoop() {
        int argc = 0;

        pybind11::gil_scoped_release release;
        return wxEntry(argc, (char **)nullptr);
    }

    bool ScheduleTask(TaskPtr task, int miliseconds, bool repeated) {
        auto timer = new __wxGLTimer(task, miliseconds, repeated);
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

TermUIPtr CreateDefaultTermUI()
{
    if (!wxApp::GetInstance())
        wxCreateApp();
    return TermUIPtr{ new DefaultTermUI()};
}

TaskPtr CreateShowContextWindowTask() {
    return TaskPtr { new __ShowContextWindowTask() };
}
