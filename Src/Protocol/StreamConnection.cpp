#include "Argenta.h"

#include "StreamConnection.h"


namespace argenta
{

StreamConnection::StreamConnection()
    : m_session(nullptr)
    , m_conn(nullptr)
{
}

StreamConnection::~StreamConnection()
{
}

void StreamConnection::ConnectionWillStart(NetSession *s)
{
    m_session = s;
}

IReceiver *StreamConnection::CreateConnectionSuccess(NetConnection *conn)
{
    m_conn = conn;
    return this;
}

void StreamConnection::CreateConnectionFail()
{
}

void StreamConnection::DestoryConnection(NetConnection *conn)
{
    m_conn = nullptr;
}

ngx_int_t StreamConnection::Recv(u_char *buf, size_t len, ngx_addr_t *addr)
{
    ngx_str_t str = {len, buf};
    ArgentaInfo("recv stream data:%V, addr:%V", &str, &addr->name);
    return NGX_OK;
}

} // namespace argenta

