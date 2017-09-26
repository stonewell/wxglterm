#pragma once

#include "multiple_instance_plugin.h"

class TermUI : public virtual MultipleInstancePlugin {
public:
    virtual void Refresh() = 0;
    virtual void Show() = 0;
};
