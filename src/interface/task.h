#pragma once

#include "multiple_instance_plugin.h"

class Task : public virtual MultipleInstancePlugin {
public:
    virtual void Run() = 0;
    virtual void Cancel() = 0;
    virtual bool IsCancelled() = 0;
};
