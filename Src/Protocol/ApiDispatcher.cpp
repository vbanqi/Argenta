#include "Argenta.h"

#include "ApiDispatcher.h"

#include "Config.h"
#include "HttpRequest.h"
#include "MemPool.h"
#include "RequestManager.h"
#include "NetInterface.h"

#define RESPONSE_ERR_TEXT "{\"errcode\":%d, \"errmsg\":\"%s\"}"
#define RESPONSE_OK_TEXT "{\"errcode\":0, \"sid\":\"%V\"}"
#define RESPONSE_OK_NO_SID "{\"errcode\":0}" 
#define RECORD_URI "/convert"
#define RECORD_START "/offer"

namespace argenta
{

ApiDispatcher::ApiDispatcher()
{
}

ApiDispatcher::~ApiDispatcher()
{
}

ngx_int_t ApiDispatcher::OnRequest(ngx_http_request_t *r)
{
    ngx_int_t rc = NGX_DECLINED;
    ArgentaInfo("recieve the data:%V", &r->uri);
    if (ngx_strncmp(r->uri.data, RECORD_URI RECORD_START, r->uri.len) == 0) {
    }
    return rc;
}

} // namespace argenta

