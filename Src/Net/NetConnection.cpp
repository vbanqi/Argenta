#include "Argenta.h"

#include "NetConnection.h"

#include "Config.h"
#include "IConnection.h"
#include "MemPool.h"
#include "IReceiver.h"


namespace argenta
{


NetConnection::NetConnection(ngx_connection_t *conn)
    : m_curAddr(nullptr)
    , m_conn(conn)
    , m_connInternal(nullptr)
    , m_receiver(nullptr)
    , m_queueCount(0)
{
    ngx_queue_init(&m_pktQueue);
}

NetConnection::~NetConnection()
{
    ngx_queue_t *que = &m_pktQueue;
    while(!ngx_queue_empty(que)) {
        ngx_queue_t *q = ngx_queue_head(que);
        PacketQuene *pq = ngx_queue_data(q, PacketQuene, m_queue);
        ngx_queue_remove(q);
        delete[] pq->m_buf->start;
        delete pq->m_buf;
        delete pq;
    }
}

void NetConnection::Initlize(IConnection *conn)
{
    m_connInternal = conn;
    if (conn) {
        m_receiver = conn->CreateConnectionSuccess(this);
    }
}

void NetConnection::Destory()
{
    if (m_connInternal) {
        m_connInternal->DestoryConnection(this);
    }
}

ngx_int_t NetConnection::Send(u_char *data, size_t len, ngx_addr_t *addr)
{
    //assert(m_conn->type == SOCK_DGRAM);
    if (m_queueCount >= 500) {
        ArgentaWarn("-----------warn-------------the udp queue count is %d", m_queueCount);
        return NGX_ERROR;
    }
    m_queueCount++;
    PacketQuene *queue = new PacketQuene;
    ngx_memzero(queue, sizeof(PacketQuene));
    
    if (m_conn->type == SOCK_DGRAM && addr != nullptr) {
        queue->m_addr = addr;
    }

    ngx_buf_t *buf = new ngx_buf_t;
    ngx_memzero(buf, sizeof(ngx_buf_t));
    buf->start = new u_char[len + 1];
    ngx_memzero(buf->start, len + 1);
    buf->pos = buf->last =  buf->start;
    buf->end = buf->start + len;
    ngx_memcpy(buf->start, data, len);
    buf->last = buf->start + len;
    queue->m_buf = buf;
    ngx_queue_insert_tail(&m_pktQueue, &queue->m_queue);
    m_conn->write->handler(m_conn->write);
    return NGX_OK;
}

/**
 * retrun NGX_AGAIN : the data is not recv done,continue;
 *        NGX_OK    : the data is processed of all;
 *        value     : the data is processed of value;
 *        
 **/
ngx_int_t NetConnection::Recv(u_char *buf, size_t size)
{
    //ngx_str_t p = {size, buf};
    //ArgentaInfo("recv data:%V", &p);
    if (m_receiver) {
        ngx_addr_t addr;
        addr.name = m_conn->addr_text;
        addr.sockaddr = m_conn->sockaddr;
        addr.socklen = m_conn->socklen;
        return m_receiver->Recv(buf, size, &addr);
    }
    return NGX_ERROR;
}

/**
 * return NGX_DONE || NGX_OK : the data is send done; It is possble call more times
 * return NGX_ERROR          : will close the connection
 *        value              : the data's length
 **/
ngx_int_t NetConnection::SendBuf(ngx_buf_t **buf)
{
    if (ngx_queue_empty(&m_pktQueue)) {
        //ArgentaDebug("send data done");
        return NGX_DONE;
    }
    ngx_queue_t *q = ngx_queue_head(&m_pktQueue);
    PacketQuene *pq = ngx_queue_data(q, PacketQuene, m_queue);
    *buf = pq->m_buf;
    
    if (m_conn->type == SOCK_DGRAM && pq->m_addr != nullptr) {
        *m_conn->sockaddr = *pq->m_addr->sockaddr;
        m_conn->socklen = pq->m_addr->socklen;
        m_conn->addr_text = pq->m_addr->name;
    }
    
    return NGX_OK;
}

void NetConnection::SendDone(size_t size)
{
    ngx_queue_t *q = ngx_queue_head(&m_pktQueue);
    PacketQuene *pq = ngx_queue_data(q, PacketQuene, m_queue);
    pq->m_buf->pos += size;
    int ll = pq->m_buf->last - pq->m_buf->pos;
    assert(ll >=0);
    if (ll == 0) {
        //ArgentaWarn("-----------warn-------------remove the udp data %d into queue", size);
        ngx_queue_remove(q);
        
        delete[] pq->m_buf->start;
        delete pq->m_buf;
        delete pq;
    }
    m_queueCount--;
}
} // namespace argenta

