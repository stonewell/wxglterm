#include "plugin_base.h"

PluginBase::PluginBase(const char * name, const char * description, uint32_t version) :
    m_Name(name)
    , m_Description(description)
    , m_Version(version)
    , m_Context{}
    , m_PluginConfig{}
    , m_Debug{false}
{
}
