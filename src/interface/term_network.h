#pragma once

#include "plugin.h"

class TermNetwork : public virtual Plugin {
public:
    virtual void Disconnect() = 0;
};
