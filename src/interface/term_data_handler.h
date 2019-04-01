#pragma once

#include "multiple_instance_plugin.h"
#include <vector>

class TermDataHandler : public MultipleInstancePlugin {
public:
    virtual void OnData(const std::vector<unsigned char> & data, size_t data_len) = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
};
