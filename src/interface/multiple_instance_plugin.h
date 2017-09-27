#pragma once

#include "plugin.h"
#include <memory>

class MultipleInstancePlugin : public virtual Plugin {
public:
    MultipleInstancePlugin() = default;
    virtual ~MultipleInstancePlugin() = default;

public:
    virtual std::shared_ptr<MultipleInstancePlugin> NewInstance() = 0;
};
