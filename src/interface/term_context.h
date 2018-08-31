#pragma once

#include "plugin_context.h"

#include "term_handles.h"

class TermContext : public virtual Context {
public:
    TermContext() = default;
    virtual ~TermContext() = default;

public:
    virtual TermBufferPtr GetTermBuffer() const = 0;
    virtual void SetTermBuffer(TermBufferPtr term_buffer) = 0;
    virtual TermWindowPtr GetTermWindow() const = 0;
    virtual void SetTermWindow(TermWindowPtr term_window) = 0;
    virtual TermNetworkPtr GetTermNetwork() const = 0;
    virtual void SetTermNetwork(TermNetworkPtr term_network) = 0;
    virtual TermDataHandlerPtr GetTermDataHandler() const = 0;
    virtual void SetTermDataHandler(TermDataHandlerPtr term_data_handler) = 0;

    virtual TermColorThemePtr GetTermColorTheme() const = 0;
    virtual void SetTermColorTheme(TermColorThemePtr term_color_theme) = 0;
    virtual InputHandlerPtr GetInputHandler() const = 0;
    virtual void SetInputHandler(InputHandlerPtr term_color_theme) = 0;
};
