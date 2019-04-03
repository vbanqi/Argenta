#ifndef __ARGENTA_NET_INTERFACE_H_INCLUDED__
#define __ARGENTA_NET_INTERFACE_H_INCLUDED__
#include "NetSession.h"

namespace argenta
{

class IConnection;
class IRequestCleanUp;
class NetConnection;
class NetSession;
class NetInterface
{
public:
	NetInterface();
	virtual ~NetInterface();

    ngx_int_t CreateConnection(ngx_argenta_stream_session_t *session);
    void DestoryConnection(NetSession *session);
    void OnDestoryConnection(ngx_argenta_stream_session_t *session);
    
    void CreateTcpConnectionPeer(const ngx_str_t *url, IConnection *obj, const ngx_str_t *bind = nullptr);
    void CreateTcpConnectionPeerSsl(const ngx_str_t *url, IConnection *conn, const ngx_str_t *bind = nullptr);
    void CreateUdpConnectionPeer(const ngx_str_t *url, IConnection *obj, const ngx_str_t *bind = nullptr);
    //void CreateUdpConnection(const ngx_str_t *url, IConnection *obj);

    ngx_int_t ConnectionWillStart(ngx_argenta_stream_session_t *ses, NetSession *ns= nullptr);
    void CreateConnectionFail(void *obj);
    ngx_int_t AddHttpCleanUp(ngx_http_request_t *r, IRequestCleanUp *cleanup);

    void OnHttpResponse(ngx_http_request_t *r, ngx_int_t ret, ngx_chain_t *ch, size_t len);
    void OnHttpStaticResponse(ngx_http_request_t *r);
}; // class NetInterface

#define SNet Singleton<NetInterface>::GetInstance()
} // namespace argenta

#endif // ifndef __ARGENTA_NET_INTERFACE_H_INCLUDED__

