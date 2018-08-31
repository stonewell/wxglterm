#pragma once

#include <string>

#include "plugin.h"
#include "plugin_context.h"
#include "app_config.h"

class PluginBase : public virtual Plugin {
public:
    PluginBase(const char * name, const char * description, uint32_t version);
    virtual ~PluginBase() = default;

public:
    const char * GetName() override {
        return m_Name.c_str();
    }
    const char * GetDescription() override {
        return m_Description.c_str();
    }

    uint32_t GetVersion() override {
        return m_Version;
    }

    virtual void InitPlugin(ContextPtr context,
                    AppConfigPtr plugin_config) override {
        m_Context = context;
        m_PluginConfig = plugin_config;
        bool app_debug = context->GetAppConfig()->GetEntryBool("app_debug", false);
        m_Debug = plugin_config->GetEntryBool("debug", app_debug);
    }
    ContextPtr GetPluginContext() const override {
        return m_Context;
    }
    AppConfigPtr GetPluginConfig() const override {
        return m_PluginConfig;
    }

private:
    std::string m_Name;
    std::string m_Description;
    uint32_t m_Version;

protected:
    ContextPtr m_Context;
    AppConfigPtr m_PluginConfig;
    bool m_Debug;
};
