#include <pybind11/embed.h>
#include <iostream>
#include <string>

namespace py = pybind11;

#include "app_config_impl.h"
#include "load_module.h"

#if !defined(_WIN32) || defined(__GNUC__)
#pragma GCC visibility push(hidden)
#endif

class AppConfigImpl : public virtual AppConfig {
public:
    AppConfigImpl();
    virtual ~AppConfigImpl() = default;

public:
    virtual std::string GetEntry(const char * path, const char * default_value) override;
    virtual int64_t GetEntryInt64(const char * path, int64_t default_value) override;
    virtual uint64_t GetEntryUInt64(const char * path, uint64_t default_value) override;
    virtual bool GetEntryBool(const char * path, bool default_value) override;

    virtual bool LoadFromFile(const char * file_path) override;
    virtual bool LoadFromString(const char * data) override;

private:
    void ResetDefaults();

    bool m_bLoaded;
    py::object m_AppConfig;
};

std::shared_ptr<AppConfig> g_AppConfig {new AppConfigImpl()};

AppConfigImpl::AppConfigImpl()
{
    ResetDefaults();
}

std::string AppConfigImpl::GetEntry(const char * path, const char * default_value)
{
    return m_bLoaded ? m_AppConfig.attr("get")(path, py::cast(default_value)).cast<std::string>() : default_value;
}

int64_t AppConfigImpl::GetEntryInt64(const char * path, int64_t default_value)
{
    return m_bLoaded ? m_AppConfig.attr("get")(path, py::cast(default_value)).cast<int64_t>() : default_value;
}

uint64_t AppConfigImpl::GetEntryUInt64(const char * path, uint64_t default_value)
{
    return m_bLoaded ? m_AppConfig.attr("get")(path, py::cast(default_value)).cast<uint64_t>() : default_value;
}

bool AppConfigImpl::GetEntryBool(const char * path, bool default_value)
{
    return m_bLoaded ? m_AppConfig.attr("get")(path, py::cast(default_value)).cast<bool>() : default_value;
}


bool AppConfigImpl::LoadFromFile(const char * file_path)
{
    try
    {
        const char *module_content =
#include "app_config.inc"
                ;

        m_AppConfig =
                LoadPyModuleFromString(module_content,
                                       "app_config",
                                       "app_config.py").attr("load_config")(file_path);

        m_bLoaded = true;
    }
    catch(std::exception & e)
    {
        std::cerr << "load configuration from file:"
                  << file_path
                  << " failed!"
                  << std::endl
                  << e.what()
                  << std::endl;
        m_bLoaded = false;
    }
    catch(...)
    {
        std::cerr << "load configuration from file:"
                  << file_path
                  << " failed!"
                  << std::endl;
        PyErr_Print();
        m_bLoaded = false;
    }

    return m_bLoaded;
}


void AppConfigImpl::ResetDefaults()
{
    m_bLoaded = false;
}

bool AppConfigImpl::LoadFromString(const char * data)
{
    try
    {
        const char *module_content =
#include "app_config.inc"
                ;

        m_AppConfig =
                LoadPyModuleFromString(module_content,
                                       "app_config",
                                       "app_config.py").attr("load_config_from_string")(data);

        m_bLoaded = true;
    }
    catch(std::exception & e)
    {
        std::cerr << "load configuration from string:"
                  << data
                  << " failed!"
                  << std::endl
                  << e.what()
                  << std::endl;
        m_bLoaded = false;
    }
    catch(...)
    {
        std::cerr << "load configuration from string:"
                  << data
                  << " failed!"
                  << std::endl;
        PyErr_Print();
        m_bLoaded = false;
    }

    return m_bLoaded;
}

std::shared_ptr<AppConfig> CreateAppConfigFromString(const char * data)
{
    std::shared_ptr<AppConfig> config {new AppConfigImpl() };
    config->LoadFromString(data);

    return config;
}

std::shared_ptr<AppConfig> CreateAppConfigFromFile(const char * file_path)
{
    std::shared_ptr<AppConfig> config {new AppConfigImpl() };
    config->LoadFromFile(file_path);

    return config;
}

#if !defined(_WIN32) || defined(__GNUC__)
#pragma GCC visibility pop
#endif
