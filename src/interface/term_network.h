#pragma once

#include "multiple_instance_plugin.h"

class TermNetwork : public virtual MultipleInstancePlugin {
public:
    virtual void Disconnect() = 0;
    virtual void Connect(const char * host, int port, const char * user_name, const char * password) = 0;
};
