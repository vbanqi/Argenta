#ifndef __ARGENTA_MEM_POOL_H_INCLUDED__
#define __ARGENTA_MEM_POOL_H_INCLUDED__


namespace argenta
{

class MemPool
{
public:
	MemPool(size_t size);
	MemPool(ngx_pool_t *pool);
	~MemPool();
    
    void *Alloc(size_t size);
    void *Calloc(size_t size);
    int Free(void *p);
    void Reset();

    ngx_pool_t *Pool() { return m_pool; }
private:
    ngx_pool_t *m_pool;
    ngx_pool_t *m_ownPool;

}; // class MemPool

} // namespace nspr

#endif // ifndef __ARGENTA_MEM_POOL_H_INCLUDED__

