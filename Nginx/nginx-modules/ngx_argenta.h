#ifndef _NGX_ARGENTA_H_INCLUDED_
#define _NGX_ARGENTA_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <nginx.h>
#include <ngx_http.h>
#include <ngx_argenta_api_module.h>
#include <ngx_argenta_stream.h>
#include <ngx_argenta_stream_peer.h>
#include <ngx_argenta_log.h>

#define NGX_INTERNAL             0x02                                                               
#define NGX_STREAM                 0x01                                                               

typedef enum {
    SERVER,
    CLIENT
} NetType;

extern ngx_module_t ngx_argenta_stream_module;
extern ngx_module_t ngx_argenta_stream_peer_module;


#endif /* _NGX_ARGENTA_H_INCLUDED_ */

