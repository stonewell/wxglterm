#pragma once

#include <memory>

class BufferRoller {
public:
    BufferRoller() = default;
    virtual ~BufferRoller() = default;

    BufferRoller(const BufferRoller &) = delete;
    BufferRoller & operator =(const BufferRoller &) = delete;

public:
    virtual uint8_t * GetLine(uint32_t index) = 0;
    virtual void RollUp(uint32_t count) = 0;
    virtual void RollDown(uint32_t count) = 0;
    virtual void Normalize() = 0;
};

using BufferRollerPtr = std::shared_ptr<BufferRoller>;

BufferRollerPtr CreateBufferRoller(uint8_t * buf_begin, uint8_t * buf_end, size_t line_size);
