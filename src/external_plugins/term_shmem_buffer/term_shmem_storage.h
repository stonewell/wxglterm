#pragma once

#include <memory>

class TermShmemStorage {
public:
    TermShmemStorage() = default;
    virtual ~TermShmemStorage() = default;
public:
    virtual uint8_t * GetAddress() = 0;
    virtual size_t GetSize() const = 0;
};

using TermShmemStoragePtr = std::shared_ptr<TermShmemStorage>;

TermShmemStoragePtr CreateTermShmemStorage(size_t size);
