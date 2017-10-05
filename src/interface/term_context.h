#pragma once

#include "context.h"

#include "term_handles.h"

class TermContext : public virtual Context {
public:
    TermContext() = default;
    virtual ~TermContext() = default;

public:
    virtual TermBufferPtr GetTermBuffer() const = 0;
    virtual void SetTermBuffer(TermBufferPtr term_buffer) = 0;
    virtual TermUIPtr GetTermUI() const = 0;
    virtual void SetTermUI(TermUIPtr term_ui) = 0;
    virtual TermNetworkPtr GetTermNetwork() const = 0;
    virtual void SetTermNetwork(TermNetworkPtr term_network) = 0;
};
