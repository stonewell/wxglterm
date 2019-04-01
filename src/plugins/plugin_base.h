#pragma once

#include <string>

#include "plugin.h"
#include "plugin_context.h"
#include "app_config.h"

#define PLUGIN_BASE_INIT_LIST(name, description, version) \
        m_Name(name) \
        , m_Description(description) \
        , m_Version(version) \
        , m_Context{} \
        , m_PluginConfig{} \
        , m_Debug{false}

#define PLUGIN_BASE_DEFINE() PLUGIN_BASE_DEFINE_PREFIX(, , , , , )
#define PLUGIN_BASE_DEFINE_PREFIX(n_p, d_p, v_p, i_p, t_p, c_p) \
public: \
    virtual const char * n_p ## GetName() { \
        return m_Name.c_str(); \
    } \
    virtual const char * d_p ## GetDescription() { \
        return m_Description.c_str(); \
    } \
    virtual uint32_t v_p ## GetVersion() { \
        return m_Version; \
    } \
    virtual void i_p ##InitPlugin(ContextPtr context, \
                            AppConfigPtr plugin_config) { \
		m_Context = context; \
        m_PluginConfig = plugin_config; \
        bool app_debug = context->GetAppConfig()->GetEntryBool("app_debug", false); \
        m_Debug = plugin_config->GetEntryBool("debug", app_debug); \
    } \
    virtual ContextPtr t_p ## GetPluginContext() const { \
        return m_Context; \
    } \
    virtual AppConfigPtr c_p ## GetPluginConfig() const { \
        return m_PluginConfig; \
    } \
private: \
    std::string m_Name; \
    std::string m_Description; \
    uint32_t m_Version; \
protected: \
    ContextPtr m_Context; \
    AppConfigPtr m_PluginConfig; \
    bool m_Debug;
