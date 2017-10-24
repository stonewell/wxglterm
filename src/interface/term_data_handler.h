#pragma once

#include "multiple_instance_plugin.h"

class TermDataHandler : public virtual MultipleInstancePlugin {
public:
    virtual void OnData(const char * data, size_t data_len) = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
};
