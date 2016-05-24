#include "flow2d.hpp"

#include <vector>

NS_FLOW2D_BEGIN

struct GenericPool
{
    explicit GenericPool(size_t element_size, size_t chunk_size);
    virtual ~GenericPool();

    size_t      size() const;
    size_t      capacity() const;
    size_t      chunks() const;

    void        expand(size_t);
    void        reserve(size_t);
    void*       get(size_t);
    const void* get(size_t) const;

    virtual void erase(size_t) = 0;

protected:
    std::vector<char *> m_blocks;
    size_t              m_element_size;
    size_t              m_chunk_size;
    size_t              m_size = 0;
    size_t              m_capacity = 0;
};

template<typename T, size_t ChunkSize = 1024>
struct Pool : public GenericPool
{
    Pool() : GenericPool(sizeof(T), ChunkSize) {}
    virtual ~Pool() {}
    virtual void erase(size_t n) override
    {
        T *ptr = static_cast<T*>(get(n));
        ptr->~T();
    }
};

#include "pool.inl"

NS_FLOW2D_END