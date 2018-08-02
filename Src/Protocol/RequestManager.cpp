#include "Argenta.h"

#include "RequestManager.h"

#include "Config.h"

namespace argenta
{

RequestManager::RequestManager()
    : m_sequence(0)
    , m_requestCount(0)
{
}

RequestManager::~RequestManager()
{
}

int RequestManager::AddRequest(SHttpRequestPtr &req)
{
    int seq = m_sequence;
    if (m_requestPool[seq]) {
        //ArgentaAssert(false);
        ArgentaError("the reqeust is in the pool, this:%p, count:%d", this, m_requestCount);
        return NGX_ERROR;
    }
    m_requestPool[m_sequence++] = req;
    if (m_sequence >= kMaxRequest) {
        m_sequence = 0;
    }
    m_requestCount++;
    return seq;
}

SHttpRequestPtr RequestManager::GetRequest(int seq)
{
    if (seq < 0) {
        ArgentaInfo("have no request:%d-%d", seq, m_requestCount);
        return SHttpRequestPtr();
    }
    return m_requestPool[seq];
}
SHttpRequestPtr RequestManager::RemoveRequest(int seq)
{
    if (seq == -1) {
        SHttpRequestPtr p;
        return p;
    }
    SHttpRequestPtr ptr = m_requestPool[seq];
    m_requestPool[seq].reset();
    ArgentaInfo("remove the request:%d-%d", seq, m_requestCount);
    m_requestCount--;
    //ArgentaAssert(ptr);
    return ptr;
}
} // namespace argenta

