#pragma once

#include "app_config.h"
#include <memory>

extern std::shared_ptr<AppConfig> g_AppConfig;
std::shared_ptr<AppConfig> CreateAppConfigFromString(const char * data);
std::shared_ptr<AppConfig> CreateAppConfigFromFile(const char * file);
