#include "Argenta.h"

#include "MemPool.h"
#include "ngx_palloc.h"


namespace argenta
{

MemPool::MemPool(size_t size)
    : m_pool(nullptr)
    , m_ownPool(nullptr)
{
    m_ownPool = ngx_create_pool(size, ngx_cycle->log);
    m_pool = m_ownPool;
}

MemPool::MemPool(ngx_pool_t *pool)
    : m_pool(nullptr)
    , m_ownPool(nullptr)
{
    m_pool = pool;
}

MemPool::~MemPool()
{
    if (m_ownPool) {
        ngx_destroy_pool(m_ownPool);
    }
}

void *MemPool::Alloc(size_t size)
{
    return ngx_palloc(m_pool, size);
}
void *MemPool::Calloc(size_t size)
{
    return ngx_pcalloc(m_pool, size);
}
int MemPool::Free(void *p)
{
    return ngx_pfree(m_pool, p);
}
void MemPool::Reset()
{
    if (m_ownPool) {
        ngx_reset_pool(m_ownPool);
    }
}

} // namespace argenta

