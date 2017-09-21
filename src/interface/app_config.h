#pragma once

#include <string>

class AppConfig {
public:
    AppConfig() = default;
    virtual ~AppConfig() = default;

public:
    virtual std::string GetEntry(const char * path, const char * default_value) = 0;
    virtual int64_t GetEntryInt64(const char * path, int64_t default_value) = 0;
    virtual uint64_t GetEntryUInt64(const char * path, uint64_t default_value) = 0;
    virtual bool GetEntryBool(const char * path, bool default_value) = 0;

    virtual bool LoadFromFile(const char * file_path) = 0;
};
