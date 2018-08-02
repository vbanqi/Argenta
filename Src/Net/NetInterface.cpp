#include "Argenta.h"

#include "NetInterface.h"

#include "IRequestCleanUp.h"
#include "NetConnection.h"
#include "NetFactory.h"
#include "NetSession.h"
#include "IConnection.h"


namespace argenta
{

NetInterface::NetInterface()
{
}

NetInterface::~NetInterface()
{
}

void NetInterface::CreateTcpConnectionPeer(const ngx_str_t *url, IConnection *conn, const ngx_str_t *bind)
{
    ngx_argenta_connection_peer(url, bind, conn, SOCK_STREAM);
}

void NetInterface::CreateUdpConnectionPeer(const ngx_str_t *url, IConnection *conn, const ngx_str_t *bind)
{
    ngx_argenta_connection_peer(url, bind, conn, SOCK_DGRAM);
}

void NetInterface::DestoryConnection(NetSession *sess)
{
    sess->SetManual(true);//主动发起销毁连接
    if (sess->Role() == SERVER) {
        ngx_argenta_stream_finalize(sess->GetSession()->c, NGX_OK);
    }
    else {
        ngx_argenta_stream_peer_finalize(sess->GetSession(), NGX_OK);
    }
}
void NetInterface::OnDestoryConnection(ngx_argenta_stream_session_t *session)
{
    NetConnection *net = (NetConnection *)session->data;
    net->Destory();
    delete net;
}

ngx_int_t NetInterface::CreateConnection(ngx_argenta_stream_session_t *session)
{
    if (session->role == SERVER) {
        u_char *ch = (u_char *)ngx_pcalloc(session->pool, sizeof(NetSession));
        if (ch == NULL) {
            return NGX_ERROR;
        }
        NetSession *s = new(ch)NetSession(session);
        session->sess = s;
        session->iconn = Singleton<NetFactory>::GetInstance()->CreateConnection(s);
        if (ConnectionWillStart(session) == NGX_ERROR) {
            return NGX_ERROR;
        }
    }
    NetConnection *con = new NetConnection(session->c);
    session->data = con;
    con->Initlize((IConnection *)session->iconn);
    return NGX_OK;
}

ngx_int_t NetInterface::ConnectionWillStart(ngx_argenta_stream_session_t *ses, NetSession *ns)
{
    IConnection *c = (IConnection *)ses->iconn;
    NetSession *s = ns;
    if (!s) {
        u_char *ch = (u_char *)ngx_pcalloc(ses->pool, sizeof(NetSession));
        if (ch == NULL) {
            return NGX_ERROR;
        }
        s = new(ch)NetSession(ses);
        ses->sess = s;
    }
    c->ConnectionWillStart(s);
    return NGX_OK;
}

void NetInterface::CreateConnectionFail(void *conn)
{
    if (conn) {
        ((IConnection *)conn)->CreateConnectionFail();
    }
}

ngx_int_t NetInterface::AddHttpCleanUp(ngx_http_request_t *r, IRequestCleanUp *cleanup)
{
    return argenta_app_core_api_add_cleanup(r, cleanup);
}

void NetInterface::OnHttpResponse(ngx_http_request_t *r, ngx_int_t ret, ngx_chain_t *ch, size_t len)
{
    ngx_chain_t *chain = ch;
    if (len == 0) {
        ngx_buf_t buf;
        chain = ngx_alloc_chain_link(r->pool);
        chain->buf = &buf;
        ngx_memzero(&buf, sizeof(ngx_buf_t));
        buf.last_buf = 1;
        buf.last_in_chain = 1;
    }
    r->headers_out.status = ret;
    r->headers_out.content_length_n = len;
    r->headers_out.content_type = ngx_string("application/json");
    
    argenta_app_core_api_response(r, chain);
}
} // namespace argenta

