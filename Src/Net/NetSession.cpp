#include "Argenta.h"

#include "NetSession.h"


namespace argenta
{

NetSession::NetSession(ngx_argenta_stream_session_t *sid)
    : m_session(sid)
{
}

NetSession::~NetSession()
{
}

void NetSession::SetManual(bool manual)
{
    if (manual) {
        m_session->manual = 1;
    }
    else {
        m_session->manual = 0;
    }
}
int NetSession::Role()
{
    return m_session->role;
}
} // namespace argenta

