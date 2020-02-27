#include "buffer_line_mapper.h"

#include <cassert>
#include <vector>
#include <algorithm>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/anonymous_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

class BufferLineMapperImpl : public BufferLineMapper {
public:
    BufferLineMapperImpl(uint8_t * buf_begin, uint8_t * buf_end, size_t line_size);
    virtual ~BufferLineMapperImpl() = default;

    BufferLineMapperImpl(const BufferLineMapperImpl &) = delete;
    BufferLineMapperImpl & operator =(const BufferLineMapperImpl &) = delete;

public:
    virtual uint8_t * GetLine(uint32_t index);
    virtual void RollUp(uint32_t begin, uint32_t end, uint32_t count);
    virtual void RollDown(uint32_t begin, uint32_t end, uint32_t count);

private:
    uint8_t * m_OriginBegin;
    uint8_t * m_OriginEnd;
    size_t m_LineSize;

    std::vector<uint8_t *> m_LineMapper;
};

BufferLineMapperPtr CreateBufferLineMapper(uint8_t * buf_begin, uint8_t * buf_end, size_t line_size) {
    return std::make_shared<BufferLineMapperImpl>(buf_begin, buf_end, line_size);
}

BufferLineMapperImpl::BufferLineMapperImpl(uint8_t * buf_begin, uint8_t * buf_end, size_t line_size)
    : m_OriginBegin{buf_begin}
    , m_OriginEnd {buf_end}
    , m_LineSize {line_size}
    , m_LineMapper {}
{
    m_LineMapper.resize((m_OriginEnd - m_OriginBegin) / m_LineSize);

    uint8_t * line = buf_begin;

    for(std::vector<uint8_t*>::size_type i = 0; i < m_LineMapper.size(); i++) {
        m_LineMapper[i] = line;
        line += m_LineSize;
    }
}

uint8_t * BufferLineMapperImpl::GetLine(uint32_t index) {
    assert(index < m_LineMapper.size());

    return m_LineMapper[index];
}

void BufferLineMapperImpl::RollUp(uint32_t begin, uint32_t end, uint32_t count) {
    if (count >= end - begin) {
        //do nothing, since all lines roll out, no need to swap
        return;
    }

    auto it_r_begin = m_LineMapper.begin() + begin;
    auto it_r_end = m_LineMapper.begin() + end;
    auto it_r_middle = it_r_begin + count;

    std::rotate(it_r_begin, it_r_middle, it_r_end);
}

void BufferLineMapperImpl::RollDown(uint32_t begin, uint32_t end, uint32_t count) {
    if (count >= end - begin) {
        //do nothing, since all lines roll out, no need to swap
        return;
    }

    auto it_r_end = m_LineMapper.rend() - begin + 1;
    auto it_r_begin = m_LineMapper.rend() - end + 1;
    auto it_r_middle = it_r_begin + count;

    std::rotate(it_r_begin, it_r_middle, it_r_end);
}
