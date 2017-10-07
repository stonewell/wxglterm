#include "plugin_base.h"

#include "default_term_ui.h"
#include "term_window.h"

#include "main_dlg.h"

class __wxGLTermApp : public wxApp {
public:
    virtual bool OnInit() {
        return true;
    }
};

wxIMPLEMENT_APP_NO_MAIN(__wxGLTermApp);

class DefaultTermWindow : public virtual PluginBase, public virtual TermWindow {
public:
    DefaultTermWindow() :
        PluginBase("default_term_window", "default terminal window plugin", 0)
        , m_MainDlg(nullptr) {
    }

    virtual ~DefaultTermWindow() = default;

public:
    void Refresh() override {
        if (!m_MainDlg)
            return;
    }

    void Show() override {
        if (!m_MainDlg)
            m_MainDlg = new MainDialog(wxT("wxGLTerm"));

        m_MainDlg->Show(true);
    }

private:
    MainDialog * m_MainDlg;
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
};

TermUIPtr CreateDefaultTermUI()
{
    return TermUIPtr{ new DefaultTermUI()};
}
