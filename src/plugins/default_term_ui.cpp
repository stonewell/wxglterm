#include "plugin_base.h"

#include "default_term_ui.h"

#include "main_dlg.h"

class DefaultTermUI : public virtual PluginBase, public virtual TermUI {
public:
    DefaultTermUI() :
        PluginBase("term_ui", "default terminal ui plugin", 0)
        , m_MainDlg(wxT("wxGLTerm"))
    {
    }

    virtual ~DefaultTermUI() = default;

    void Refresh() override {
    }

    void Show() override {
        m_MainDlg.Show(true);
    }

    MultipleInstancePlugin * NewInstance() override {
        return new DefaultTermUI();
    }

private:
    MainDialog m_MainDlg;
};

TermUI * CreateDefaultTermUI()
{
    TermUI * termUI{ new DefaultTermUI()};

    return termUI;
}
