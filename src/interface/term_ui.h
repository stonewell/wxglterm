#pragma once

#include "plugin.h"

class TermUI : public virtual Plugin {
public:
    virtual int32_t StartMainUILoop() = 0;
    virtual TermWindowPtr CreateWindow() = 0;
};
