#include "plugin_base.h"

#include "default_term_ui.h"

#include "main_dlg.h"

static
TermUI * g_MainUI = nullptr;

class __wxGLTermApp : public wxApp {
public:
    virtual bool OnInit() {
        if (!g_MainUI)
            return false;

        g_MainUI->Show();

        return true;
    }
};

wxIMPLEMENT_APP_NO_MAIN(__wxGLTermApp);

class DefaultTermUI : public virtual PluginBase, public virtual TermUI {
public:
    DefaultTermUI() :
        PluginBase("default_term_ui", "default terminal ui plugin", 0)
        , m_MainDlg(nullptr)
    {
    }

    virtual ~DefaultTermUI() = default;

    void Refresh() override {
        if (!m_MainDlg)
            return;
    }

    void Show() override {
        if (!m_MainDlg)
            m_MainDlg = new MainDialog(wxT("wxGLTerm"));

        m_MainDlg->Show(true);
    }

    std::shared_ptr<MultipleInstancePlugin> NewInstance() override {
        return std::shared_ptr<MultipleInstancePlugin>{new DefaultTermUI()};
    }

    int32_t StartMainUILoop() {
        int argc = 0;

        g_MainUI = this;

        return wxEntry(argc, (char **)nullptr);
    }
private:
    MainDialog * m_MainDlg;
};

TermUIPtr CreateDefaultTermUI()
{
    return TermUIPtr{ new DefaultTermUI()};
}
