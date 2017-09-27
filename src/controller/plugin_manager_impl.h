#pragma once

#include "plugin_manager.h"
#include "handles.h"

#include <queue>
#include <functional>
#include <vector>
#include <string>
#include <map>

using SharedPluginPtr = std::shared_ptr<Plugin>;

using PluginList = std::forward_list<SharedPluginPtr>;
using PluginMap = std::map<std::string, PluginList>;
using HandleList = std::forward_list<Handle>;

class PluginManagerImpl : public PluginManager, public std::enable_shared_from_this<PluginManagerImpl> {
public:
    PluginManagerImpl();
    virtual ~PluginManagerImpl();

public:
    void RegisterPlugin(SharedPluginPtr plugin) override;
    void RegisterPlugin(const char * plugin_file_path) override;
    SharedPluginPtr GetPlugin(const char * plugin_name, uint64_t plugin_version_code) override;

private:
    void LoadPythonPlugin(const char * plugin_file_path);
    void LoadDylibPlugin(const char * plugin_file_path);

    HandleList m_DylibHandleList;
    PluginMap m_PluginMap;
};
