#include "term_shmem_storage.h"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/anonymous_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace impl {
class TermShmemStorageImpl : public TermShmemStorage {
public:
    TermShmemStorageImpl(size_t size)
        : m_MappedRegion{boost::interprocess::anonymous_shared_memory(size)}
    {
    }

    virtual ~TermShmemStorageImpl() = default;

public:
    virtual uint8_t * GetAddress() {
        return reinterpret_cast<uint8_t*>(m_MappedRegion.get_address());
    }

    virtual size_t GetSize() const{
        return m_MappedRegion.get_size();
    }

private:
    boost::interprocess::mapped_region m_MappedRegion;
};
}

TermShmemStoragePtr CreateTermShmemStorage(size_t size) {
    auto p = std::make_shared<impl::TermShmemStorageImpl>(size);

    return p;
}
