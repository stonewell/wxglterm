#pragma once

#include "plugin.h"

class PluginBase : public virtual Plugin {
public:
    PluginBase(const char * name, const char * description, uint32_t version) :
        m_Name(name)
        , m_Description(description)
        , m_Version(version)
        , m_Context{}
        , m_PluginConfig{}
    {
    }
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
};