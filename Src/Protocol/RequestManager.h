#ifndef __ARGENTA_REQUEST_MANAGER_H_INCLUDED__
#define __ARGENTA_REQUEST_MANAGER_H_INCLUDED__
#include "HttpRequest.h"
#define SRequestManager Singleton<RequestManager>::GetInstance()

namespace argenta
{

class RequestManager
{
public:
	RequestManager();
	virtual ~RequestManager();

    int AddRequest(SHttpRequestPtr &req);
    SHttpRequestPtr GetRequest(int seq);
    SHttpRequestPtr RemoveRequest(int seq);

private:
//TODO:
    static const int kMaxRequest = 100000;
    SHttpRequestPtr m_requestPool[kMaxRequest];
    size_t m_sequence;
    size_t m_requestCount;

}; // class RequestManager

} // namespace argenta

#endif // ifndef __ARGENTA_REQUEST_MANAGER_H_INCLUDED__

