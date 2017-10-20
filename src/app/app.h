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
    using term_ui_list_t = std::list<TermUIPtr>;

    std::shared_ptr<void> m_PyInterpreter;
    term_ui_list_t m_TermUIList;
    PluginManagerPtr m_PluginManager;

    bool DoInit();
    void Cleanup();
    void InitDefaultPlugins();
    TermUIPtr CreateTermUI(TermContextPtr term_context);
    TermContextPtr CreateTermContext();
    TermNetworkPtr CreateTermNetwork(TermContextPtr term_context);
    TermDataHandlerPtr CreateTermDataHandler(TermContextPtr term_context);
    TaskPtr CreateShowContextWindowTask(TermContextPtr term_context);
    TermBufferPtr CreateTermBuffer(TermContextPtr term_context);
    TermColorThemePtr CreateTermColorTheme(TermContextPtr term_context);
};
