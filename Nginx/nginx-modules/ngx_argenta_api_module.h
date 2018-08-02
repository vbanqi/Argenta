#ifndef _NGX_ARGENTA_API_MODULE_H_INCLUDED_
#define _NGX_ARGENTA_API_MODULE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <nginx.h>
#include <ngx_http.h>

extern void argenta_app_core_api_response(ngx_http_request_t *r, ngx_chain_t *ch);

extern ngx_int_t argenta_app_core_api_add_cleanup(ngx_http_request_t *r, void *data);
#endif /* _NGX_ARGENTA_API_MODULE_H_INCLUDED_ */

