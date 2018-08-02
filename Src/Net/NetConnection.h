#ifndef __ARGENTA_NET_CONNECTION_H_INCLUDED__
#define __ARGENTA_NET_CONNECTION_H_INCLUDED__
#include "ISender.h"

namespace argenta
{

class IConnection;
class IReceiver;
class NetConnection: public ISender
{
public:
	NetConnection(ngx_connection_t *conn);
	virtual ~NetConnection();

    virtual void Initlize(IConnection *obj);
    
    virtual void Destory();

    void SetReceiver(IReceiver *recv) { m_receiver = recv; }

    virtual ngx_int_t Send(u_char *data, size_t len, ngx_addr_t *addr = nullptr) override;

    virtual ngx_int_t Recv(u_char *buf, size_t size);
    virtual ngx_int_t SendBuf(ngx_buf_t **buf);

    virtual void SendDone(size_t size);
private:
    typedef struct {
        ngx_buf_t *m_buf;
        ngx_addr_t *m_addr;
        ngx_queue_t m_queue;
    } PacketQuene;

private:
    ngx_queue_t m_pktQueue;
    ngx_addr_t *m_curAddr;
    ngx_connection_t *m_conn;
    IConnection *m_connInternal;
    IReceiver *m_receiver;
    size_t m_queueCount;
}; // class NetConnection

} // namespace argenta

#endif // ifndef __ARGENTA_NET_CONNECTION_H_INCLUDED__

