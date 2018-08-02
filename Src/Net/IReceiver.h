#ifndef __ARGENTA_I_RECEIVER_H_INCLUDED__
#define __ARGENTA_I_RECEIVER_H_INCLUDED__


namespace argenta
{

class IReceiver
{
public:
	IReceiver() {};
	virtual ~IReceiver() {};

    virtual ngx_int_t Recv(u_char *buf, size_t len, ngx_addr_t *addr) = 0;
}; // class IReceiver

} // namespace argenta

#endif // ifndef __ARGENTA_I_RECEIVER_H_INCLUDED__

