#include <pybind11/embed.h>
#include <iostream>
#include <string>

#include "app_config_impl.h"

namespace py = pybind11;

#pragma GCC visibility push(hidden)
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

private:
    void ResetDefaults();

    bool m_Loaded;
    py::object m_AppConfig;
};

std::shared_ptr<AppConfig> g_AppConfig {new AppConfigImpl()};

AppConfigImpl::AppConfigImpl()
{
    ResetDefaults();
}

std::string AppConfigImpl::GetEntry(const char * path, const char * default_value)
{
    return m_AppConfig.attr("get")(path, py::cast(default_value)).cast<std::string>();
}

int64_t AppConfigImpl::GetEntryInt64(const char * path, int64_t default_value)
{
    return m_AppConfig.attr("get")(path, py::cast(default_value)).cast<int64_t>();
}

uint64_t AppConfigImpl::GetEntryUInt64(const char * path, uint64_t default_value)
{
    return m_AppConfig.attr("get")(path, py::cast(default_value)).cast<uint64_t>();
}

bool AppConfigImpl::GetEntryBool(const char * path, bool default_value)
{
    return m_AppConfig.attr("get")(path, py::cast(default_value)).cast<bool>();
}


bool AppConfigImpl::LoadFromFile(const char * file_path)
{
    try
    {
        py::dict locals;

        locals["config_path"]    = py::cast(file_path);

        py::object result = py::eval<py::eval_statements>(
#include "app_config.inc"
            "if not os.path.exists(config_path):\n"
            "    raise ValueError('unable to find the config file:{}'.format(config_path))\n"
            "with open(config_path) as f:\n"
            "    app_config = DictQuery(json.load(f))\n"
            ,
            py::globals(),
            locals);

        m_AppConfig = locals["app_config"];

        m_Loaded = true;
    }
    catch(std::exception & e)
    {
        std::cerr << "load configuration from file:"
                  << file_path
                  << " failed!"
                  << std::endl
                  << e.what()
                  << std::endl;
        m_Loaded = false;
    }
    catch(...)
    {
        std::cerr << "load configuration from file:"
                  << file_path
                  << " failed!"
                  << std::endl;
        PyErr_Print();
        m_Loaded = false;
    }

    return m_Loaded;
}


void AppConfigImpl::ResetDefaults()
{
    m_Loaded = false;
}

#pragma GCC visibility pop
