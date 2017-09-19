#pragma once

#include "plugin.h"

class TermUI : public virtual Plugin {
public:
    virtual void Refresh() = 0;
};
