#include "MemPool.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

namespace hola
{

MemPool::MemPool()
{

}

MemPool::~MemPool()
{
}

int MemPool::initlize(size_t size)
{
    void *pv;
    pool_t *p;
    int err;

    m_pagesize = getpagesize();
    err = posix_memalign(&pv, POOL_ALIGNMENT, size);
    if (err) {
        return -1;
    }
    p = (pool_t *)pv;

    p->d.last = (u_char *) p + sizeof(pool_t);
    p->d.end = (u_char *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(pool_t);
    p->max = (size < m_pagesize - 1) ? size : m_pagesize - 1;

    p->current = p;
    p->large = NULL;
    p->cleanup = NULL;
    m_pool = p;
    return 0;
}

int MemPool::uninitlize()
{
    pool_t          *p, *n;
    pool_large_t    *l;
    pool_cleanup_t  *c;

    for (c = m_pool->cleanup; c; c = c->next) {
        if (c->handler) {
            c->handler(c->data);
        }
    }

    for (l = m_pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    for (p = m_pool, n = m_pool->d.next; /* void */; p = n, n = n->d.next) {
        free(p);

        if (n == NULL) {
            break;
        }
    }
    return 0;
}

void *MemPool::Palloc(size_t size)
{
    if (size <= m_pool->max) {
        return PallocSmall(size, 1);
    }
    return PallocLarge(size);
}
void *MemPool::Pnalloc(size_t size)
{
    if (size <= m_pool->max) {
        return PallocSmall(size, 0);
    }

    return PallocLarge(size);
}
void *MemPool::Pcalloc(size_t size)
{
    void *p;

    p = Palloc(size);
    if (p) {
        memset(p, 0, size);
    }

    return p;

}
void *MemPool::PallocSmall(size_t size, uintptr_t align)
{
    u_char      *m;
    pool_t  *p;

    p = m_pool->current;

    do {
        m = p->d.last;

        if (align) {
            m = align_ptr(m, sizeof(unsigned long));
        }

        if ((size_t) (p->d.end - m) >= size) {
            p->d.last = m + size;

            return m;
        }

        p = p->d.next;

    } while (p);

    return PallocBlock(size);
}
void *MemPool::PallocLarge(size_t size)
{
    void              *p;
    uintptr_t         n;
    pool_large_t  *large;

    p = malloc(size);
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    for (large = m_pool->large; large; large = large->next) {
        if (large->alloc == NULL) {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) {
            break;
        }
    }

    large = (pool_large_t *)PallocSmall(sizeof(pool_large_t), 1);
    if (large == NULL) {
        free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = m_pool->large;
    m_pool->large = large;

    return p;
}
void *MemPool::PallocBlock(size_t s)
{
    int          err;
    u_char      *m;
    size_t       psize;
    pool_t  *p, *np;

    psize = (size_t) (m_pool->d.end - (u_char *) m_pool);

    err = posix_memalign((void **)&m, POOL_ALIGNMENT, s);
    if (err) {
        return NULL;
    }

    np = (pool_t *) m;

    np->d.end = m + psize;
    np->d.next = NULL;
    np->d.failed = 0;

    m += sizeof(pool_data_t);
    m = align_ptr(m, sizeof(unsigned long));
    np->d.last = m + s;

    for (p = m_pool->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            m_pool->current = p->d.next;
        }
    }

    p->d.next = np;

    return m;
}
int MemPool::Pfree(void *p)
{    
    pool_large_t  *l;

    for (l = m_pool->large; l; l = l->next) {
        if (p == l->alloc) {
            free(l->alloc);
            l->alloc = NULL;

            return 0;
        }
    }

    return -1;
}
pool_cleanup_t *
MemPool::PoolCleanupAdd(size_t size)
{
    pool_cleanup_t  *c;

    c = Palloc(sizeof(pool_cleanup_t));
    if (c == NULL) {
        return NULL;
    }

    if (size) {
        c->data = Palloc(size);
        if (c->data == NULL) {
            return NULL;
        }

    } else {
        c->data = NULL;
    }

    c->handler = NULL;
    c->next = m_pool->cleanup;

    m_pool->cleanup = c;

    return c;
}
} // namespace hola

