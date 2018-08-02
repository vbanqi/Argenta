#ifndef __ARGENTA_NET_FACTORY_H_INCLUDED__
#define __ARGENTA_NET_FACTORY_H_INCLUDED__

namespace argenta
{

class IConnection;
class NetSession;
class NetFactory
{
public:
	NetFactory();
	virtual ~NetFactory();

    IConnection *CreateConnection(NetSession *session);
    void DestoryConnection(IConnection *con);

}; // class NetFactory

} // namespace argenta

#endif // ifndef __ARGENTA_NET_FACTORY_H_INCLUDED__

