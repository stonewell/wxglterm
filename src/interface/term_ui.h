#pragma once

#include "multiple_instance_plugin.h"
#include <memory>

class TermUI : public virtual MultipleInstancePlugin {
public:
    virtual void Refresh() = 0;
    virtual void Show() = 0;
};

using TermUIPtr = std::shared_ptr<TermUI>;
