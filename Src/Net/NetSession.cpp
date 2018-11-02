#include "Argenta.h"

#include "NetSession.h"


namespace argenta
{

NetSession::NetSession(ngx_argenta_stream_session_t *sid)
    : m_session(sid)
{
    struct sockaddr_in *in;
    if (sid->local_sockaddr) {
        in = (struct sockaddr_in *)sid->local_sockaddr;
        ngx_sock_ntop(sid->local_sockaddr, sid->local_socklen, m_localIp, 32, 1);
        m_localPort = in->sin_port;
    }
    if (sid->sockaddr) {
        in = (struct sockaddr_in *)sid->sockaddr;
        u_char *dt = ngx_cpymem(m_peerIp, sid->peer_name->data, sid->peer_name->len);
        *dt = 0;
        m_peerPort = in->sin_port;
    }
    ArgentaInfo("create connection session localaddr:%s, port:%d, peeraddr:%s, port:%d", m_localIp, m_localPort, m_peerIp, m_peerPort);
}

NetSession::~NetSession()
{
}

void NetSession::SetManual(bool manual)
{
    if (manual) {
        m_session->manual = 1;
    }
    else {
        m_session->manual = 0;
    }
}
int NetSession::Role()
{
    return m_session->role;
}
} // namespace argenta

