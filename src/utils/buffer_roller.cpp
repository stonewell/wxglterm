#include "buffer_roller.h"

#include <cassert>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/anonymous_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

class BufferRollerImpl : public BufferRoller {
public:
    BufferRollerImpl(uint8_t * buf_begin, uint8_t * buf_end, size_t line_size);
    virtual ~BufferRollerImpl() = default;

    BufferRollerImpl(const BufferRollerImpl &) = delete;
    BufferRollerImpl & operator =(const BufferRollerImpl &) = delete;

public:
    virtual uint8_t * GetLine(uint32_t index);
    virtual void RollUp(uint32_t count);
    virtual void RollDown(uint32_t count);
    virtual void Normalize();

private:
    uint8_t * m_OriginBegin;
    uint8_t * m_OriginEnd;
    uint8_t * m_Begin;
    size_t m_LineSize;
};

BufferRollerPtr CreateBufferRoller(uint8_t * buf_begin, uint8_t * buf_end, size_t line_size) {
    return std::make_shared<BufferRollerImpl>(buf_begin, buf_end, line_size);
}

BufferRollerImpl::BufferRollerImpl(uint8_t * buf_begin, uint8_t * buf_end, size_t line_size)
    : m_OriginBegin{buf_begin}
    , m_OriginEnd {buf_end}
    , m_Begin {buf_begin}
    , m_LineSize {line_size}
{
}

uint8_t * BufferRollerImpl::GetLine(uint32_t index) {
    uint8_t * line = m_Begin + index * m_LineSize;

    assert(line >= m_Begin);

    if (line >= m_OriginEnd) {
        line = m_OriginBegin + (line - m_OriginEnd);
        assert(line >= m_OriginBegin && line < m_Begin);
    }

    return line;
}

void BufferRollerImpl::Normalize() {
    if (m_Begin == m_OriginBegin) return;

    assert(m_Begin > m_OriginBegin && m_Begin < m_OriginEnd);

    if ((m_Begin - m_OriginBegin) > (m_OriginEnd - m_Begin)) {
        size_t size = (m_OriginEnd - m_Begin);
        boost::interprocess::mapped_region region {boost::interprocess::anonymous_shared_memory(size)};

        memcpy(region.get_address(), m_Begin, size);
        memmove(m_OriginBegin + size, m_OriginBegin, m_Begin - m_OriginBegin);
        memcpy(m_OriginBegin, region.get_address(), size);
    } else {
        size_t size = (m_Begin - m_OriginBegin);
        boost::interprocess::mapped_region region {boost::interprocess::anonymous_shared_memory(size)};

        memcpy(region.get_address(), m_OriginBegin, size);
        memmove(m_OriginBegin, m_OriginBegin + size, m_OriginEnd - m_Begin);
        memcpy(m_OriginEnd - size, region.get_address(), size);
    }
}

void BufferRollerImpl::RollUp(uint32_t count) {
    assert(m_Begin > m_OriginBegin && m_Begin < m_OriginEnd);

    uint8_t * line = m_Begin + count * m_LineSize;

    assert(line >= m_Begin);

    if (line >= m_OriginEnd) {
        line = m_OriginBegin + (line - m_OriginEnd);
        assert(line >= m_OriginBegin && line <= m_Begin);

        if (line == m_Begin)
            line = m_OriginBegin;
    }

    m_Begin = line;
}

void BufferRollerImpl::RollDown(uint32_t count) {
    assert(m_Begin > m_OriginBegin && m_Begin < m_OriginEnd);

    size_t s = m_Begin - m_OriginBegin;
    size_t r_s = count * m_LineSize;

    if (s >= r_s) {
        m_Begin -= r_s;
    } else {
        assert(m_OriginEnd > reinterpret_cast<uint8_t*>(r_s - s));

        auto line = m_OriginEnd - (r_s - s);

        if (line == m_Begin)
            line = m_OriginBegin;
        m_Begin = line;
    }
}
