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

    const u_char *LocalIp() const { return m_localIp; }
    const u_char *peerIp() const { return m_localIp; }
    int LocalPort() const { return m_localPort; }
    int PeerPort() const { return m_peerPort; }

private:
    ngx_argenta_stream_session_t *m_session;
    u_char m_localIp[32] = {0};
    int m_localPort;
    u_char m_peerIp[32] = {0};
    int m_peerPort;

}; // class NetSession

} // namespace argenta

#endif // ifndef __ARGENTA_NET_SESSION_H_INCLUDED__

