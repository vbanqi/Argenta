#ifndef __ARGENTA_I_CONNECTION_H_INCLUDED__
#define __ARGENTA_I_CONNECTION_H_INCLUDED__
#include "NetSession.h"

namespace argenta
{
    class NetConnection;
    class NetSession;
    class IReceiver;
    class IConnection {
        public:
            virtual ~IConnection() {}
            virtual void ConnectionWillStart(NetSession *s) = 0;
            virtual IReceiver *CreateConnectionSuccess(NetConnection *conn) = 0;
            virtual void CreateConnectionFail() = 0;
            virtual void DestoryConnection(NetConnection *conn) = 0;
            //virtual ngx_int_t OnRecv(u_char *buf, size_t len) = 0;
    };

} // namespace argenta

#endif // ifndef __ARGENTA_I_CONNECTION_H_INCLUDED__

