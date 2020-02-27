#pragma once

#include <memory>

class BufferLineMapper {
public:
    BufferLineMapper() = default;
    virtual ~BufferLineMapper() = default;

    BufferLineMapper(const BufferLineMapper &) = delete;
    BufferLineMapper & operator =(const BufferLineMapper &) = delete;

public:
    virtual uint8_t * GetLine(uint32_t index) = 0;
    virtual void RollUp(uint32_t begin, uint32_t end, uint32_t count) = 0;
    virtual void RollDown(uint32_t begin, uint32_t end, uint32_t count) = 0;
};

using BufferLineMapperPtr = std::shared_ptr<BufferLineMapper>;

BufferLineMapperPtr CreateBufferLineMapper(uint8_t * buf_begin, uint8_t * buf_end,
                                           size_t line_size);
