#ifndef _NGX_ARGENTA_STREAM_PEER_H_INCLUDED_
#define _NGX_ARGENTA_STREAM_PEER_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream_upstream.h>
#include "ngx_argenta_stream.h"

typedef struct {
    size_t                           buffer_size;

    size_t                           tries;
    ngx_msec_t                       connect_timeout;

    size_t                           send_cache;
    size_t                           recv_cache;
    size_t                           send_chain_count;

    ngx_addr_t                      *local;

    void                           **main_conf;
    void                           **srv_conf;
#if (NGX_STREAM_SSL)
    ngx_flag_t                       ssl_enable;
    ngx_flag_t                       ssl_session_reuse;
    ngx_uint_t                       ssl_protocols;
    ngx_str_t                        ssl_ciphers;
    ngx_str_t                        ssl_name;
    ngx_flag_t                       ssl_server_name;

    ngx_flag_t                       ssl_verify;
    ngx_uint_t                       ssl_verify_depth;
    ngx_str_t                        ssl_trusted_certificate;
    ngx_str_t                        ssl_crl;
    ngx_str_t                        ssl_certificate;
    ngx_str_t                        ssl_certificate_key;
    ngx_array_t                     *ssl_passwords;

    ngx_ssl_t                       *ssl;
#endif

    ngx_stream_upstream_srv_conf_t  *upstream;
} ngx_argenta_stream_peer_srv_conf_t;

#define ngx_argenta_stream_peer_get_conf(module)   (ngx_cycle)->conf_ctx[module.index]

extern void ngx_argenta_connection_peer(const ngx_str_t *url, const ngx_str_t *bind, void *data, int type);

extern void ngx_argenta_stream_peer_finalize(ngx_argenta_stream_session_t *s, ngx_int_t rc);
#endif
