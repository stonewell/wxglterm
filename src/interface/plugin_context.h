#pragma once

#include "multiple_instance_plugin.h"

class Context : public MultipleInstancePlugin {
public:
    Context() = default;
    virtual ~Context() = default;

public:
    virtual AppConfigPtr GetAppConfig() const = 0;
    virtual void SetAppConfig(AppConfigPtr app_config) = 0;
};
