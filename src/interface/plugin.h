#pragma once

#include "term_handles.h"
#include "app_config.h"

class Plugin {
public:
    Plugin() = default;
    virtual ~Plugin() = default;

public:
    virtual const char * GetName() = 0;
    virtual const char * GetDescription() = 0;
    virtual uint32_t GetVersion() = 0;
    virtual void InitPlugin(ContextPtr context,
                            AppConfigPtr plugin_config) = 0;
};
