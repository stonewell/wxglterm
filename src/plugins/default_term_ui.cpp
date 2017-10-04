#include "plugin_base.h"

#include "default_term_ui.h"

#include "main_dlg.h"

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

private:
    MainDialog * m_MainDlg;
};

TermUI * CreateDefaultTermUI()
{
    TermUI * termUI{ new DefaultTermUI()};

    return termUI;
}
