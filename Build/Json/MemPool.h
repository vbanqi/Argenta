#ifndef __HOLA_MEM_POOL_H_INCLUDED__
#define __HOLA_MEM_POOL_H_INCLUDED__
#include <memory>


namespace hola
{

#define align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

#define POOL_ALIGNMENT 16

typedef struct pool_s        pool_t;
typedef void (*pool_cleanup_pt)(void *data);
typedef struct pool_cleanup_s  pool_cleanup_t;



struct pool_cleanup_s {
    pool_cleanup_pt   handler;
    void                 *data;
    pool_cleanup_t   *next;
};


typedef struct pool_large_s  pool_large_t;

struct pool_large_s {
    pool_large_t     *next;
    void                 *alloc;
};


typedef struct {
    u_char               *last;
    u_char               *end;
    pool_t           *next;
    uintptr_t            failed;
} pool_data_t;


struct pool_s {
    pool_data_t       d;
    size_t                max;
    pool_t           *current;
    pool_large_t     *large;
    pool_cleanup_t   *cleanup;
};


class MemPool
{
public:
	MemPool();
	virtual ~MemPool();

    int initlize(size_t size);

    int uninitlize();

    void *Palloc(size_t s);
    void *Pnalloc(size_t s);

    void *Pcalloc(size_t size);

    int Pfree(void *p);
    
    void PoolCleanupAdd(size_t size, pool_cleanup_t *);
private:
    void *PallocSmall(size_t s, uintptr_t align);
    void *PallocLarge(size_t s);
    void *PallocBlock(size_t s);

    pool_t *m_pool;
    uintptr_t m_pagesize;
}; // class MemPool

} // namespace hola

#endif // ifndef __HOLA_MEM_POOL_H_INCLUDED__

