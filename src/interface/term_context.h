#pragma once

#include "context.h"
#include <memory>

class TermBuffer;
class TermUI;
class TermNetwork;

class TermContext : public virtual Context {
public:
    TermContext() = default;
    virtual ~TermContext() = default;

public:
    virtual std::shared_ptr<TermBuffer> GetTermBuffer() const = 0;
    virtual std::shared_ptr<TermUI> GetTermUI() const = 0;
    virtual std::shared_ptr<TermNetwork> GetTermNetwork() const = 0;
};
