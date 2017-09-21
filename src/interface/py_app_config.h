#pragma once

#include "app_config.h"

template<class AppConfigBase = AppConfig>
class PyAppConfig : public virtual AppConfigBase {
public:
    using AppConfigBase::AppConfigBase;

public:
    std::string GetEntry(const char * path, const char * default_value) override {
        PYBIND11_OVERLOAD_PURE_NAME(std::string, AppConfigBase, "get_entry", GetEntry, path, default_value);
    }

    int64_t GetEntryInt64(const char * path, int64_t default_value) override {
        PYBIND11_OVERLOAD_PURE_NAME(int64_t, AppConfigBase, "get_entry_int64", GetEntryInt64, path, default_value);
    }

    uint64_t GetEntryUInt64(const char * path, uint64_t default_value) override {
        PYBIND11_OVERLOAD_PURE_NAME(uint64_t, AppConfigBase, "get_entry_uint64", GetEntryInt64, path, default_value);
    }

    bool GetEntryBool(const char * path, bool default_value) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, AppConfigBase, "get_entry_bool", GetEntryBool, path, default_value);
    }

    bool LoadFromFile(const char * file_path) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, AppConfigBase, "load_from_file", LoadFromFile, file_path);
    }
};
