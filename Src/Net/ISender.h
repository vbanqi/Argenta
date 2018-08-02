#ifndef __ARGENTA_I_SENDER_H_INCLUDED__
#define __ARGENTA_I_SENDER_H_INCLUDED__


namespace argenta
{

class ISender
{
public:
	ISender() {};
	virtual ~ISender() {};

    virtual ngx_int_t Send(u_char *buf, size_t len, ngx_addr_t *addr = nullptr) = 0;
}; // class ISender

} // namespace argenta

#endif // ifndef __ARGENTA_I_SENDER_H_INCLUDED__

