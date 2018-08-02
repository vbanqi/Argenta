#ifndef __ARGENTA_STREAM_CONNECTION_H_INCLUDED__
#define __ARGENTA_STREAM_CONNECTION_H_INCLUDED__
#include "IConnection.h"
#include "IReceiver.h"

namespace argenta
{

class StreamConnection: public IConnection, public IReceiver
{
public:
	StreamConnection();
	virtual ~StreamConnection();

    virtual void ConnectionWillStart(NetSession *s);
    virtual IReceiver *CreateConnectionSuccess(NetConnection *conn);
    virtual void CreateConnectionFail();
    virtual void DestoryConnection(NetConnection *conn);
    virtual ngx_int_t Recv(u_char *buf, size_t len, ngx_addr_t *addr);
private:
    NetSession *m_session;
    NetConnection *m_conn;

}; // class StreamConnection

} // namespace argenta

#endif // ifndef __ARGENTA_STREAM_CONNECTION_H_INCLUDED__

