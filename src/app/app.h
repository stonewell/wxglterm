#pragma once

#include <wx/wx.h>
#include <memory>
#include <list>

#include "term_ui.h"
#include "plugin_manager.h"

class wxGLTermApp
{
public:
    bool Run(int argc, char ** argv);
    virtual ~wxGLTermApp();

private:
    using term_ui_list_t = std::list<std::shared_ptr<TermUI>>;

    std::shared_ptr<void> m_PyInterpreter;
    term_ui_list_t m_TermUIList;
    std::shared_ptr<PluginManager> m_PluginManager;

    bool DoInit();
    void Cleanup();
    void InitDefaultPlugins();
    std::shared_ptr<TermUI> CreateTermUI();
};
