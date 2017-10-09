#pragma once

#include "plugin.h"

class TermUI : public virtual Plugin {
public:
    virtual int32_t StartMainUILoop() = 0;
    virtual TermWindowPtr CreateWindow() = 0;
    virtual bool ScheduleTask(TaskPtr task, int miliseconds, bool repeated) = 0;
};
