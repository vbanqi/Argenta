#include "Argenta.h"

#include "NetFactory.h"

#include "IConnection.h"
#include "StreamConnection.h"

namespace argenta
{

NetFactory::NetFactory()
{
}

NetFactory::~NetFactory()
{
}

IConnection *NetFactory::CreateConnection(NetSession *ses)
{
    (void)ses;
    IConnection *conn = new StreamConnection();
    return conn;
}
void NetFactory::DestoryConnection(IConnection *con)
{
    delete con;
}
} // namespace argenta

