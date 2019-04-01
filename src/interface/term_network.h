#pragma once

#include "multiple_instance_plugin.h"
#include <vector>

class TermNetwork : public MultipleInstancePlugin {
public:
    virtual void Disconnect() = 0;
    virtual void Connect(const char * host, int port, const char * user_name, const char * password) = 0;
    virtual void Send(const std::vector<unsigned char> & data, size_t n) = 0;
    virtual void Resize(uint32_t row, uint32_t col) = 0;
};
