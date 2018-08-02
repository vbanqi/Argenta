#include "Argenta.h"

#include "HttpRequest.h"

#include "MemPool.h"
#include "ErrorType.h"
#include "NetInterface.h"
#include "RequestManager.h"

#define RESPONSE_ERR_TEXT "{\"errcode\":%d, \"errmsg\":\"%s\"}"
#define RESPONSE_OK_TEXT "{\"errcode\":0, \"sid\":\"%V\"}"
#define RESPONSE_OK_NO_SID "{\"errcode\":0}" 
#define RESPONSE_OK_DATA "{\"errcode\":0, \"sid\":\"%V\", \"data\":%V}" 
#define RESPONSE_SIZE 4096

namespace argenta
{

HttpRequest::HttpRequest( ngx_http_request_t *r, ngx_msec_t timeout)
    : m_pool(new MemPool(4096))
    , m_refCount(0)
    , m_maxRefCount(0)
    , m_sequence(-1)
    , m_timeOut(timeout)
    , m_request(r)
    , m_sid(nullptr)
    , m_timeOutListener(nullptr)
{
    ngx_memzero(&m_response, sizeof(ngx_str_t));
    ngx_memzero(&m_timeOutTimer, sizeof(ngx_event_t));
    m_timeOutTimer.log = ngx_cycle->log;
    m_timeOutTimer.data = this;
    m_timeOutTimer.handler = HttpRequest::OnTimeOut;
    ngx_add_timer(&m_timeOutTimer, m_timeOut);
    AddResponseRef();
}

HttpRequest::~HttpRequest()
{
    if (m_request) {
        ResponseToClient();
    }
    if (m_timeOutTimer.timer_set) {
        ngx_del_timer(&m_timeOutTimer);
    }
    ArgentaInfo("recycle the request:%d", m_sequence);
    delete m_pool;
}
void HttpRequest::SetSid(ngx_str_t *sid)
{
    m_sid = sid;
}
void HttpRequest::AddStrData(ngx_str_t *str)
{
    m_response.data = (u_char *)m_pool->Calloc(str->len);
    ngx_memcpy(m_response.data, str->data, str->len);
    m_response.len = str->len;
}

void HttpRequest::AddResponseRef(int count)
{
    m_maxRefCount += count;
    ArgentaAssert(m_maxRefCount < kMaxRef);
    m_refCount += count;
}

ngx_int_t HttpRequest::OnResponse(int code)
{
    m_refCount--;
    m_errorCode[m_refCount] = code;
    ArgentaError("on response error:%d, %d, %d", code, m_refCount, m_maxRefCount);
    if (m_refCount <= 0) {
        ResponseToClient();
        return NGX_OK;
    }
    return NGX_AGAIN;
}

void HttpRequest::OnInteraptThenResponse(int code)
{
    m_refCount--;
    m_errorCode[m_refCount] = code;
    ResponseToClient();
}

void HttpRequest::ResponseToClient()
{
    bool haveError = false, haveTimeOut = false;
    int errCode = API_ERROR_NO_ERROR;
    ngx_int_t ret = 0;
    
    for (int i = 0; i < m_maxRefCount; i++) {
        if (m_errorCode[i] == API_ERROR_TIME_OUT) {
            errCode = API_ERROR_TIME_OUT;
            haveTimeOut = true;
            break;
        }
    }
    if (!haveTimeOut) {
        for (int i = 0; i < m_maxRefCount; i++) {
            if (m_errorCode[i] != API_ERROR_NO_ERROR) {
                haveError = true;
                errCode |= m_errorCode[i];
            }
        }
    }
    //ArgentaError("on response error:seq %d, haveError %d, errorCode %d, maxrefcount %d", m_sequence, haveError,  errCode, m_maxRefCount);
    if (!haveError && !haveTimeOut) {
        ret = ResponseToClient(m_request, API_ERROR_NO_ERROR);
    }
    else {
        ret = ResponseToClient(m_request, (ErrorType)errCode);
        if (ret == NGX_AGAIN) {
            ret = ResponseToClient(m_request, API_ERROR_INTER_ERROR);
        }
    }
    if (ret == NGX_ERROR || ret == NGX_AGAIN) {
        Singleton<NetInterface>::GetInstance()->OnHttpResponse(m_request, NGX_HTTP_BAD_REQUEST, nullptr, 0);
    }
    SRequestManager->RemoveRequest(m_sequence);
    if (m_timeOutTimer.timer_set) {
        ngx_del_timer(&m_timeOutTimer);
    }
    m_sequence = -1;
    m_request = nullptr;
}

ngx_int_t HttpRequest::ResponseToClient(ngx_http_request_t *r, ErrorType type)
{
    ngx_chain_t ch;
    const ApiError *err;
    const size_t resSize = 1024;
    if (m_request == nullptr) {
        return NGX_ERROR;
    }
    ngx_buf_t *buf = ngx_create_temp_buf(r->pool, resSize);
    if (!buf) {
        ArgentaError("alloc response buf error!");
        return NGX_ERROR;
    }

    bool error = false;
    for (int i = 0; i < g_apiErrorEntityLength; i++) {
        if (type == g_ApiErrorEntity[i].type) {
            err = &g_ApiErrorEntity[i];
            error = true;
        }
    }
    if (!error) {
        ArgentaError("the error type is not define!");
        return NGX_AGAIN;
    }

    ngx_memzero(&ch, sizeof(ngx_chain_t));
    buf->temporary = 1;
    buf->last_buf = 1;
    buf->last_in_chain = 1;
    ch.buf = buf;

    if (type == API_ERROR_NO_ERROR) {
        if(m_sid == nullptr) {
            ngx_str_t s = ngx_string("");
            buf->last = ngx_snprintf(buf->start, resSize, RESPONSE_OK_NO_SID, &s);
        }
        else if (m_response.len != 0) {
            buf->last = ngx_snprintf(buf->start, resSize, RESPONSE_OK_DATA, m_sid, &m_response);
        }
        else {
            buf->last = ngx_snprintf(buf->start, resSize, RESPONSE_OK_TEXT, m_sid);
        }
        Singleton<NetInterface>::GetInstance()->OnHttpResponse(r, NGX_HTTP_OK, &ch, buf->last - buf->pos);
    } else {
        ArgentaError("response to client:%s", err->msg);
        buf->last = ngx_snprintf(buf->start, resSize, RESPONSE_ERR_TEXT, type, err->msg);
        Singleton<NetInterface>::GetInstance()->OnHttpResponse(r, NGX_HTTP_BAD_REQUEST, &ch, buf->last - buf->pos);
    }
    m_request = nullptr;
    if (m_timeOutTimer.timer_set) {
        ngx_del_timer(&m_timeOutTimer);
    }
    return NGX_OK;
}

void HttpRequest::TimeOut()
{
    OnInteraptThenResponse(API_ERROR_TIME_OUT);
    if (m_timeOutListener) {
        m_timeOutListener->OnRequestTimeOut(m_sequence);
    }
}

void HttpRequest::OnTimeOut(ngx_event_t *ev)
{
    HttpRequest *req = (HttpRequest *)ev->data;
    SHttpRequestPtr r = SRequestManager->GetRequest(req->Sequence());
    r->TimeOut();
}
} // namespace argenta

