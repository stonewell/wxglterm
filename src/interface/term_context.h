#pragma once

#include "context.h"

class TermBuffer;
class TermUI;
class TermNetwork;

class TermContext : public virtual Context {
public:
    TermContext() = default;
    virtual ~TermContext() = default;

public:
    virtual TermBuffer * GetTermBuffer() const = 0;
    virtual TermUI * GetTermUI() const = 0;
    virtual TermNetwork * GetTermNetwork() const = 0;
};
