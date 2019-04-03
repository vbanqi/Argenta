#ifndef _NGX_ARGENTA_STREAM_H_INCLUDED_
#define _NGX_ARGENTA_STREAM_H_INCLUDED_

#include "ngx_stream.h"

typedef struct {
    ngx_msec_t                       api_timeout;

    size_t                  pool_size;
    size_t                  send_cache;
    size_t                  recv_cache;
    size_t                  send_chain_count;
    ngx_int_t               type;

    void                   **main_conf;
    void                   **srv_conf;
} ngx_argenta_stream_srv_conf_t;

typedef struct {
    // connection
    ngx_connection_t       *c;
    ngx_connection_t       *uc;

    void                   *data;

    ngx_buf_t              *recv_buf;
    ngx_buf_t              *send_buf;
    size_t                  recv_buf_size;
    size_t                  send_buf_size;

    u_char                  udp_first;
    u_char                  role;

    ngx_int_t               type;
    //peer connection
    ngx_url_t               *url;
    ngx_stream_upstream_t   *upstream;
    ngx_pool_t              *pool;
    ngx_log_t               *log;

    void                   **main_conf;
    void                   **srv_conf; //data
    void                    *iconn;
    void                    *sess;
    //peer addr
    struct sockaddr         *sockaddr;
    socklen_t                socklen;
    ngx_str_t               *peer_name;
    struct sockaddr         *local_sockaddr;
    socklen_t                local_socklen;
    ngx_int_t                close:1;
    ngx_int_t                manual:1;
    ngx_int_t                ssl:1;
} ngx_argenta_stream_session_t;

typedef struct argenta_addrs_s argenta_addrs_t;

struct argenta_addrs_s {
    size_t                  socklen;
    struct sockaddr        *sockaddr;
    ngx_str_t               addr_text;
    argenta_addrs_t           *next;
};

void ngx_argenta_stream_finalize(ngx_connection_t *c, ngx_int_t rc);

extern void ngx_argenta_create_udp_connection(const ngx_str_t *url, void *data);

#endif
