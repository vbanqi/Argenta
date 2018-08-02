#ifndef __ARGENTA_HTTP_REQUEST_H_INCLUDED__
#define __ARGENTA_HTTP_REQUEST_H_INCLUDED__
#include "ErrorType.h"
#include <memory>

namespace argenta
{

class RequestTimeOutListener {
    public:
        virtual void OnRequestTimeOut(int seq) = 0; 
};

class MemPool;
class HttpRequest
{
public:
	HttpRequest(ngx_http_request_t *r, ngx_msec_t tiem_out);
	virtual ~HttpRequest();

    void SetTimeOutListener(RequestTimeOutListener *timeout) { m_timeOutListener = timeout; }
    void SetSid(ngx_str_t *sid);

    void AddStrData(ngx_str_t *str);

    int Sequence() { return m_sequence; }
    void SetSequence(int seq) { m_sequence = seq; }

    void AddResponseRef(int count = 1);

    ngx_int_t OnResponse(int code);

    void OnInteraptThenResponse(int code);

    ngx_int_t ResponseToClient(ngx_http_request_t *r, ErrorType type);

    void TimeOut();

    int RefrenceCount() { return m_refCount; }

    static void OnTimeOut(ngx_event_t *ev);
private:
    void ResponseToClient();
private:
    static const int kMaxRef = 64;
    MemPool *m_pool;
    int m_errorCode[kMaxRef] = { API_ERROR_NO_ERROR };
    int m_refCount;
    int m_maxRefCount;
    int m_sequence;
    ngx_msec_t m_timeOut;
    ngx_http_request_t *m_request;
    ngx_str_t *m_sid;
    ngx_str_t m_response;
    ngx_event_t m_timeOutTimer;
    RequestTimeOutListener *m_timeOutListener;
}; // class HttpRequest

typedef std::shared_ptr<HttpRequest> SHttpRequestPtr;
} // namespace argenta

#endif // ifndef __ARGENTA_HTTP_REQUEST_H_INCLUDED__

