#ifndef __ARGENTA_NET_SESSION_H_INCLUDED__
#define __ARGENTA_NET_SESSION_H_INCLUDED__


namespace argenta
{
class NetSession
{
public:
	NetSession(ngx_argenta_stream_session_t *sid);
	virtual ~NetSession();
    
    ngx_argenta_stream_session_t *GetSession() { return m_session; }

    int Role();

    void SetManual(bool manual);
private:
    ngx_argenta_stream_session_t *m_session;

}; // class NetSession

} // namespace argenta

#endif // ifndef __ARGENTA_NET_SESSION_H_INCLUDED__

