#pragma once

#include "plugin.h"

class TermWindow : public virtual Plugin {
public:
    TermWindow() = default;
    virtual ~TermWindow() = default;

public:
    virtual void Refresh() = 0;
    virtual void Show() = 0;
    virtual void SetWindowTitle(const std::string & title) = 0;
};
