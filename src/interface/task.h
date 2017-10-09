#pragma once

#include "plugin.h"

class Task : public virtual Plugin {
public:
    virtual void Run() = 0;
    virtual void Cancel() = 0;
    virtual bool IsCancelled() = 0;
};
