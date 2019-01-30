
/*
 * Copyright (C) Roman Arutyunyan
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_stream.h>
#include "ngx_argenta.h"
#include "ngx_argenta_stream_peer.h"

extern void argenta_init_connection(ngx_argenta_stream_session_t *s);
extern void argenta_destory_connection(ngx_argenta_stream_session_t *s);
extern ngx_int_t argenta_start_connection(ngx_argenta_stream_session_t *s);
extern void argenta_create_connection_fail(void *obj);

extern void argenta_app_core_tcp_recv(ngx_event_t *rev);
extern void argenta_app_core_tcp_send(ngx_event_t *wev);
extern void argenta_app_core_udp_recv(ngx_event_t *rev);
extern void argenta_app_core_udp_send(ngx_event_t *wev);

void ngx_argenta_stream_peer_finalize(ngx_argenta_stream_session_t *s, ngx_int_t rc);

void ngx_argenta_connection_peer(const ngx_str_t *url, const ngx_str_t *bind, void *iconn, int type);

static char *ngx_argenta_stream_peer_init(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static void ngx_argenta_stream_peer_connect(ngx_argenta_stream_session_t *s);
static void ngx_argenta_stream_peer_init_upstream(ngx_argenta_stream_session_t *s);
static void ngx_argenta_stream_peer_connect_handler(ngx_event_t *ev);
static ngx_int_t ngx_argenta_stream_peer_test_connect(ngx_connection_t *c);
static void ngx_argenta_stream_peer_next_upstream(ngx_argenta_stream_session_t *s);

static void *ngx_argenta_stream_peer_create_srv_conf(ngx_conf_t *cf);
static char *ngx_argenta_stream_peer_merge_srv_conf(ngx_conf_t *cf, void *parent,
    void *child);
/*
 *static char *ngx_argenta_stream_peer_bind(ngx_conf_t *cf, ngx_command_t *cmd,
 *    void *conf);
 */

static ngx_url_t *ngx_argenta_parse_addr(ngx_pool_t *pool, const ngx_str_t *u);

ngx_argenta_stream_peer_srv_conf_t *argenta_peer_conf;
#if (NGX_STREAM_SSL)
static char *ngx_argenta_stream_peer_ssl_password_file(ngx_conf_t *cf,
    ngx_command_t *cmd, void *conf);
static void ngx_argenta_stream_peer_ssl_init_connection(ngx_argenta_stream_session_t *s);
static void ngx_argenta_stream_peer_ssl_handshake(ngx_connection_t *pc);
static ngx_int_t ngx_argenta_stream_peer_ssl_name(ngx_argenta_stream_session_t *s);
static ngx_int_t ngx_argenta_stream_peer_set_ssl(ngx_conf_t *cf,
    ngx_argenta_stream_peer_srv_conf_t *pscf);


static ngx_conf_bitmask_t  ngx_argenta_stream_peer_ssl_protocols[] = {
    { ngx_string("SSLv2"), NGX_SSL_SSLv2 },
    { ngx_string("SSLv3"), NGX_SSL_SSLv3 },
    { ngx_string("TLSv1"), NGX_SSL_TLSv1 },
    { ngx_string("TLSv10Sv1.1"), NGX_SSL_TLSv1_1 },
    { ngx_string("TLSv1.2"), NGX_SSL_TLSv1_2 },
    { ngx_null_string, 0 }
};

#endif

static ngx_command_t  ngx_argenta_stream_peer_command[] = {

    { ngx_string("argenta_peer"),
      NGX_STREAM_SRV_CONF|NGX_CONF_NOARGS,
      ngx_argenta_stream_peer_init,
      NGX_STREAM_SRV_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("argenta_peer_buffer_size"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, buffer_size),
      NULL },

    { ngx_string("argenta_peer_connect_timeout"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, connect_timeout),
      NULL },

    { ngx_string("argenta_peer_recv_buffer"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, recv_cache),
      NULL },

    { ngx_string("argenta_peer_send_buffer"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, send_cache),
      NULL },

    { ngx_string("argenta_peer_send_chain_count"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, send_chain_count),
      NULL },

    { ngx_string("argenta_peer_tries"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, tries),
      NULL },


#if (NGX_STREAM_SSL)

    { ngx_string("argenta_ssl"),
      NGX_STREAM_SRV_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_enable),
      NULL },

    { ngx_string("argenta_ssl_session_reuse"),
      NGX_STREAM_SRV_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_session_reuse),
      NULL },

    { ngx_string("argenta_ssl_protocols"),
      NGX_STREAM_SRV_CONF|NGX_CONF_1MORE,
      ngx_conf_set_bitmask_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_protocols),
      &ngx_argenta_stream_peer_ssl_protocols },

    { ngx_string("argenta_ssl_ciphers"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_ciphers),
      NULL },

    { ngx_string("argenta_ssl_name"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_name),
      NULL },

    { ngx_string("argenta_ssl_server_name"),
      NGX_STREAM_SRV_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_server_name),
      NULL },

    { ngx_string("argenta_ssl_verify"),
      NGX_STREAM_SRV_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_verify),
      NULL },

    { ngx_string("argenta_ssl_verify_depth"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_verify_depth),
      NULL },

    { ngx_string("argenta_ssl_trusted_certificate"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_trusted_certificate),
      NULL },

    { ngx_string("argenta_ssl_crl"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_crl),
      NULL },

    { ngx_string("argenta_ssl_certificate"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_certificate),
      NULL },

    { ngx_string("argenta_ssl_certificate_key"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_argenta_stream_peer_srv_conf_t, ssl_certificate_key),
      NULL },

    { ngx_string("argenta_ssl_password_file"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      ngx_argenta_stream_peer_ssl_password_file,
      NGX_STREAM_SRV_CONF_OFFSET,
      0,
      NULL },

#endif

      ngx_null_command
};


static ngx_stream_module_t  ngx_argenta_stream_peer_module_ctx = {
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    ngx_argenta_stream_peer_create_srv_conf,      /* create server configuration */
    ngx_argenta_stream_peer_merge_srv_conf        /* merge server configuration */
};


ngx_module_t  ngx_argenta_stream_peer_module = {
    NGX_MODULE_V1,
    &ngx_argenta_stream_peer_module_ctx,          /* module context */
    ngx_argenta_stream_peer_command,             /* module directives */
    NGX_STREAM_MODULE,                     /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static char *
ngx_argenta_stream_peer_init(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{

    ngx_stream_conf_ctx_t *ctx = cf->ctx;
    argenta_peer_conf = conf;

    argenta_peer_conf->main_conf = ctx->main_conf;
    argenta_peer_conf->srv_conf = ctx->srv_conf;
    return NGX_CONF_OK;
}

void 
ngx_argenta_connection_peer(const ngx_str_t *url, const ngx_str_t *bind, void *iconn, int type)
{
    ngx_argenta_stream_peer_srv_conf_t     *spscf;
    ngx_pool_t                          *pool;
    ngx_argenta_stream_session_t             *s;
    ngx_stream_upstream_t               *us;
    ngx_url_t                           *u, *b = NULL;

    spscf = argenta_peer_conf;

    pool = ngx_create_pool(spscf->buffer_size, ngx_cycle->log);
    if (!pool) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "alloc connection buffer error");
        goto create_fail;
    }

    s = ngx_pcalloc(pool, sizeof(ngx_argenta_stream_session_t));
    if (!s) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "alloc ngx_argenta_stream_session_t fail");
        goto create_fail;
    }
    //s->net_type = CLIENT;
    s->main_conf = spscf->main_conf;
    s->srv_conf = spscf->srv_conf;
    s->pool = pool;
    s->log = ngx_cycle->log;
    s->iconn = iconn;

    u = ngx_argenta_parse_addr(pool, url); 
    if (u == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "parser url error");
        goto create_fail;
    }
    if (bind) {
        b = ngx_argenta_parse_addr(pool, bind); 
        if (b == NULL) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0, "parser bind url error");
            goto create_fail;
        }

        s->local_sockaddr = b->addrs->sockaddr;
        s->local_socklen = b->addrs->socklen;
    }
    s->url = u;

    us = ngx_pcalloc(pool, sizeof(ngx_stream_upstream_t));
    if (us == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "alloc ngx_stream_upstream_t error");
        goto create_fail;
    }
    
    s->recv_buf = ngx_create_temp_buf(s->pool, spscf->recv_cache);
    if (!s->recv_buf) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "alloc ngx_stream_upstream_t error");
        goto create_fail;
    }
    s->send_buf = ngx_create_temp_buf(s->pool, spscf->send_cache);
    if (!s->send_buf) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "alloc ngx_stream_upstream_t error");
        goto create_fail;
    }

    s->type = type;
    s->role = CLIENT;
    s->sockaddr = u->addrs->sockaddr;
    s->socklen = u->addrs->socklen;
    s->peer_name = &u->addrs->name;

    if (argenta_start_connection(s) == NGX_ERROR) {
        goto create_fail;
    }

    s->send_buf->last_buf = 1;
    s->send_buf->last_in_chain = 1;
    s->send_buf->recycled = 1;

    s->send_buf_size = spscf->send_cache;
    s->recv_buf_size = spscf->recv_cache;

    s->upstream = us;

    us->peer.log = ngx_cycle->log;
    us->peer.log_error = NGX_ERROR_ERR;
    us->peer.type = type;
    us->peer.tries = spscf->tries;
    us->peer.start_time = ngx_current_msec;

    if (bind) {
        us->peer.local = b->addrs;
    }
    us->peer.sockaddr = u->addrs->sockaddr;
    us->peer.socklen = u->addrs->socklen;
    us->peer.name = &u->host;
    us->peer.get = ngx_event_get_peer;

    ngx_log_debug2(NGX_LOG_DEBUG_STREAM, s->log, 0, "argenta start connect:url:%V, %p", us->peer.name, u);
    /*
     *us->upstream_buf = *ubuf;
     *us->downstream_buf = *dbuf;
     */
    ngx_argenta_stream_peer_connect(s);
    return;
create_fail:
    argenta_create_connection_fail(iconn);
    if (pool) {
        ngx_destroy_pool(pool);
    }
    return;
}

static ngx_url_t *ngx_argenta_parse_addr(ngx_pool_t *pool, const ngx_str_t *url)
{
    ngx_int_t        ret;
    ngx_url_t       *u;

    u = ngx_pcalloc(pool, sizeof(ngx_url_t));
    if (!u) {
        return NULL;
    }

    u->url.data = url->data;
    u->url.len = url->len;

    u->default_port = 80;
    u->uri_part = 1;

    ret = ngx_parse_url(pool, u);
    if (ret != NGX_OK) {
        return NULL;
    }
    return u;
}
static void
ngx_argenta_stream_peer_connect(ngx_argenta_stream_session_t *s)
{
    ngx_int_t                     rc;
    ngx_connection_t             *pc;
    ngx_stream_upstream_t        *u;
    ngx_argenta_stream_peer_srv_conf_t  *pscf;

    u = s->upstream;

    ngx_log_debug1(NGX_LOG_ERR, s->log, 0, "argenta start connect:url:%V", &s->url->url);
    rc = ngx_event_connect_peer(&u->peer);

    ngx_log_debug2(NGX_LOG_ERR, s->log, 0, "argenta connected:url:%V, rc=%i", &s->url->url, rc);
    if (rc == NGX_ERROR) {
        ngx_argenta_stream_peer_finalize(s, NGX_ERROR);
        goto create_fail;
    }

    if (rc == NGX_BUSY) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "no live upstreams");
        ngx_argenta_stream_peer_finalize(s, NGX_DECLINED);
        goto create_fail;
    }

    if (rc == NGX_DECLINED) {
        ngx_argenta_stream_peer_next_upstream(s);
        return;
    }

    /* rc == NGX_OK || rc == NGX_AGAIN || rc == NGX_DONE */

    pc = u->peer.connection;
    s->c = pc;

    pc->sockaddr = s->sockaddr;
    pc->socklen = s->socklen;
    pc->data = s;
    pc->log = s->log;
    pc->pool = s->pool;
    pc->read->log = s->log;
    pc->write->log = s->log;

    ngx_log_debug0(NGX_LOG_INFO, s->log, 0, "argenta connected");
    if (rc != NGX_AGAIN) {
        ngx_argenta_stream_peer_init_upstream(s);
        return;
    }

    pc->read->handler = ngx_argenta_stream_peer_connect_handler;
    pc->write->handler = ngx_argenta_stream_peer_connect_handler;

    pscf = ngx_stream_get_module_srv_conf(s, ngx_argenta_stream_peer_module);

    ngx_add_timer(pc->write, pscf->connect_timeout);
    return;

create_fail:
    argenta_create_connection_fail(s->iconn);
    return;

}


static void
ngx_argenta_stream_peer_init_upstream(ngx_argenta_stream_session_t *s)
{
    int                           tcp_nodelay;
    ngx_connection_t             *pc;
    ngx_stream_upstream_t        *u;
    ngx_stream_core_srv_conf_t   *cscf;
    ngx_argenta_stream_peer_srv_conf_t  *pscf;

    u = s->upstream;
    pc = u->peer.connection;

    cscf = ngx_stream_get_module_srv_conf(s, ngx_stream_core_module);

    if (pc->type == SOCK_STREAM
        && cscf->tcp_nodelay
        && pc->tcp_nodelay == NGX_TCP_NODELAY_UNSET)
    {
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, pc->log, 0, "tcp_nodelay");

        tcp_nodelay = 1;

        if (setsockopt(pc->fd, IPPROTO_TCP, TCP_NODELAY,
                       (const void *) &tcp_nodelay, sizeof(int)) == -1)
        {
            ngx_connection_error(pc, ngx_socket_errno,
                                 "setsockopt(TCP_NODELAY) failed");
            ngx_argenta_stream_peer_next_upstream(s);
            return;
        }

        pc->tcp_nodelay = NGX_TCP_NODELAY_SET;
    }
/*
 *
 *    if (u->proxy_protocol) {
 *        if (ngx_argenta_stream_peer_send_proxy_protocol(s) != NGX_OK) {
 *            return;
 *        }
 *
 *        u->proxy_protocol = 0;
 *    }
 */

    pscf = ngx_stream_get_module_srv_conf(s, ngx_argenta_stream_peer_module);

#if (NGX_STREAM_SSL)
    if (pc->type == SOCK_STREAM && pscf->ssl && pc->ssl == NULL) {
        ngx_argenta_stream_peer_ssl_init_connection(s);
        return;
    }
#endif

/*
 *    if (u->upstream_buf.start == NULL) {
 *        p = ngx_pnalloc(s->pool, pscf->buffer_size);
 *        if (p == NULL) {
 *            ngx_argenta_stream_peer_finalize(s, NGX_ERROR);
 *            return;
 *        }
 *
 *        u->upstream_buf.start = p;
 *        u->upstream_buf.end = p + pscf->buffer_size;
 *        u->upstream_buf.pos = p;
 *        u->upstream_buf.last = p;
 *    }
 */

/*
 *    if (c->type == SOCK_DGRAM) {
 *        s->received = c->buffer->last - c->buffer->pos;
 *        u->downstream_buf = *c->buffer;
 *
 *        if (pscf->responses == 0) {
 *            pc->read->ready = 0;
 *            pc->read->eof = 1;
 *        }
 *    }
 */

    u->connected = 1;

    if(pc->type == SOCK_DGRAM) {
        pc->read->handler = argenta_app_core_udp_recv;
        pc->write->handler = argenta_app_core_udp_send;
    }
    else {
        pc->read->handler = argenta_app_core_tcp_recv;
        pc->write->handler = argenta_app_core_tcp_send;
    }

    if (pc->read->ready || pc->read->eof) {
        ngx_post_event(pc->read, &ngx_posted_events);
    }
    ngx_log_error(NGX_LOG_ERR, pc->log, 0,
            "argenta connect:%V successfully fd:%d", &s->url->url, pc->fd);
    argenta_init_connection(s);

    if (ngx_handle_write_event(pc->read, 0) != NGX_OK) {
        ngx_argenta_stream_finalize(pc, NGX_ERROR);
        ngx_log_error(NGX_LOG_ERR, pc->log, 0,
                "add init read handler error");
        return;
    }
    //ngx_argenta_stream_peer_process(s, 0, 1);
}


#if (NGX_STREAM_SSL)
static char *
ngx_argenta_stream_peer_ssl_password_file(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf)
{
    ngx_argenta_stream_peer_srv_conf_t *pscf = conf;

    ngx_str_t  *value;

    if (pscf->ssl_passwords != NGX_CONF_UNSET_PTR) {
        return "is duplicate";
    }

    value = cf->args->elts;

    pscf->ssl_passwords = ngx_ssl_read_password_file(cf, &value[1]);

    if (pscf->ssl_passwords == NULL) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static void
ngx_argenta_stream_peer_ssl_init_connection(ngx_argenta_stream_session_t *s)
{
    ngx_int_t                     rc;
    ngx_connection_t             *pc;
    ngx_stream_upstream_t        *u;
    ngx_argenta_stream_peer_srv_conf_t  *pscf;

    u = s->upstream;

    pc = u->peer.connection;

    pscf = ngx_stream_get_module_srv_conf(s, ngx_argenta_stream_peer_module);

    if (ngx_ssl_create_connection(pscf->ssl, pc, NGX_SSL_BUFFER|NGX_SSL_CLIENT)
        != NGX_OK)
    {
        ngx_argenta_stream_peer_finalize(s, NGX_ERROR);
        argenta_create_connection_fail(s->iconn);
        return;
    }

    if (pscf->ssl_server_name || pscf->ssl_verify) {
        if (ngx_argenta_stream_peer_ssl_name(s) != NGX_OK) {
            ngx_argenta_stream_peer_finalize(s, NGX_ERROR);
            argenta_create_connection_fail(s->iconn);
            return;
        }
    }

    if (pscf->ssl_session_reuse) {
        if (u->peer.set_session(&u->peer, u->peer.data) != NGX_OK) {
            ngx_argenta_stream_peer_finalize(s, NGX_ERROR);
            argenta_create_connection_fail(s->iconn);
            return;
        }
    }

    s->c->log->action = "SSL handshaking to upstream";

    rc = ngx_ssl_handshake(pc);

    if (rc == NGX_AGAIN) {

        if (!pc->write->timer_set) {
            ngx_add_timer(pc->write, pscf->connect_timeout);
        }

        pc->ssl->handler = ngx_argenta_stream_peer_ssl_handshake;
        return;
    }

    ngx_argenta_stream_peer_ssl_handshake(pc);
}


static void
ngx_argenta_stream_peer_ssl_handshake(ngx_connection_t *pc)
{
    long                          rc;
    ngx_argenta_stream_session_t         *s;
    ngx_stream_upstream_t        *u;
    ngx_argenta_stream_peer_srv_conf_t  *pscf;

    s = pc->data;

    pscf = ngx_stream_get_module_srv_conf(s, ngx_argenta_stream_peer_module);

    if (pc->ssl->handshaked) {

        if (pscf->ssl_verify) {
            rc = SSL_get_verify_result(pc->ssl->connection);

            if (rc != X509_V_OK) {
                ngx_log_error(NGX_LOG_ERR, pc->log, 0,
                              "upstream SSL certificate verify error: (%l:%s)",
                              rc, X509_verify_cert_error_string(rc));
                goto failed;
            }

            u = s->upstream;

            if (ngx_ssl_check_host(pc, &u->ssl_name) != NGX_OK) {
                ngx_log_error(NGX_LOG_ERR, pc->log, 0,
                              "upstream SSL certificate does not match \"%V\"",
                              &u->ssl_name);
                goto failed;
            }
        }

        if (pscf->ssl_session_reuse) {
            u = s->upstream;
            u->peer.save_session(&u->peer, u->peer.data);
        }

        if (pc->write->timer_set) {
            ngx_del_timer(pc->write);
        }

        ngx_argenta_stream_peer_init_upstream(s);

        return;
    }

failed:

    ngx_argenta_stream_peer_next_upstream(s);
}


static ngx_int_t
ngx_argenta_stream_peer_ssl_name(ngx_argenta_stream_session_t *s)
{
    u_char                       *p, *last;
    ngx_str_t                     name;
    ngx_stream_upstream_t        *u;
    ngx_argenta_stream_peer_srv_conf_t  *pscf;

    pscf = ngx_stream_get_module_srv_conf(s, ngx_argenta_stream_peer_module);

    u = s->upstream;

    name = pscf->ssl_name;

    if (name.len == 0) {
        name = pscf->upstream->host;
    }

    if (name.len == 0) {
        goto done;
    }

    /*
     * ssl name here may contain port, strip it for compatibility
     * with the http module
     */

    p = name.data;
    last = name.data + name.len;

    if (*p == '[') {
        p = ngx_strlchr(p, last, ']');

        if (p == NULL) {
            p = name.data;
        }
    }

    p = ngx_strlchr(p, last, ':');

    if (p != NULL) {
        name.len = p - name.data;
    }

    if (!pscf->ssl_server_name) {
        goto done;
    }

#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME

    /* as per RFC 6066, literal IPv4 and IPv6 addresses are not permitted */

    if (name.len == 0 || *name.data == '[') {
        goto done;
    }

    if (ngx_inet_addr(name.data, name.len) != INADDR_NONE) {
        goto done;
    }

    /*
     * SSL_set_tlsext_host_name() needs a null-terminated string,
     * hence we explicitly null-terminate name here
     */

    p = ngx_pnalloc(s->c->pool, name.len + 1);
    if (p == NULL) {
        return NGX_ERROR;
    }

    (void) ngx_cpystrn(p, name.data, name.len + 1);

    name.data = p;

    ngx_log_debug1(NGX_LOG_DEBUG_STREAM, s->c->log, 0,
                   "upstream SSL server name: \"%s\"", name.data);

    if (SSL_set_tlsext_host_name(u->peer.connection->ssl->connection, name.data)
        == 0)
    {
        ngx_ssl_error(NGX_LOG_ERR, s->c->log, 0,
                      "SSL_set_tlsext_host_name(\"%s\") failed", name.data);
        return NGX_ERROR;
    }

#endif

done:

    u->ssl_name = name;

    return NGX_OK;
}

#endif

static void
ngx_argenta_stream_peer_connect_handler(ngx_event_t *ev)
{
    ngx_connection_t      *c;
    ngx_argenta_stream_session_t  *s;

    c = ev->data;
    s = c->data;

    if (ev->timedout) {
        ngx_log_error(NGX_LOG_ERR, c->log, NGX_ETIMEDOUT, "upstream timed out");
        ngx_argenta_stream_peer_next_upstream(s);
        return;
    }

    ngx_del_timer(c->write);

    ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0,
                   "argenta stream peer connect upstream");

    if (ngx_argenta_stream_peer_test_connect(c) != NGX_OK) {
        ngx_argenta_stream_peer_next_upstream(s);
        return;
    }

    ngx_argenta_stream_peer_init_upstream(s);
}


static ngx_int_t
ngx_argenta_stream_peer_test_connect(ngx_connection_t *c)
{
    int        err;
    socklen_t  len;

#if (NGX_HAVE_KQUEUE)

    if (ngx_event_flags & NGX_USE_KQUEUE_EVENT)  {
        err = c->write->kq_errno ? c->write->kq_errno : c->read->kq_errno;

        if (err) {
            (void) ngx_connection_error(c, err,
                                    "kevent() reported that connect() failed");
            return NGX_ERROR;
        }

    } else
#endif
    {
        err = 0;
        len = sizeof(int);

        /*
         * BSDs and Linux return 0 and set a pending error in err
         * Solaris returns -1 and sets errno
         */

        if (getsockopt(c->fd, SOL_SOCKET, SO_ERROR, (void *) &err, &len)
            == -1)
        {
            err = ngx_socket_errno;
        }

        if (err) {
            (void) ngx_connection_error(c, err, "connect() failed");
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}


static void
ngx_argenta_stream_peer_next_upstream(ngx_argenta_stream_session_t *s)
{
    ngx_msec_t                    timeout;
    ngx_connection_t             *pc;
    ngx_stream_upstream_t        *u;
    ngx_argenta_stream_peer_srv_conf_t  *pscf;

    ngx_log_debug0(NGX_LOG_DEBUG_STREAM, s->log, 0,
                   "argenta stream proxy next upstream");

    u = s->upstream;

    /*
     *if (u->peer.sockaddr) {
     *    u->peer.free(&u->peer, u->peer.data, NGX_PEER_FAILED);
     *    u->peer.sockaddr = NULL;
     *}
     */

    pscf = ngx_stream_get_module_srv_conf(s, ngx_argenta_stream_peer_module);

    timeout = pscf->connect_timeout;

    u->peer.tries --;

    if (u->peer.tries == 0
        || (timeout && ngx_current_msec - u->peer.start_time >= timeout))
    {
        ngx_argenta_stream_peer_finalize(s, NGX_DECLINED);
        return;
    }

    pc = u->peer.connection;

    if (pc) {
        ngx_log_debug1(NGX_LOG_DEBUG_STREAM, s->log, 0,
                       "close proxy upstream connection: %d", pc->fd);

#if (NGX_STREAM_SSL)
        if (pc->ssl) {
            pc->ssl->no_wait_shutdown = 1;
            pc->ssl->no_send_shutdown = 1;

            (void) ngx_ssl_shutdown(pc);
        }
#endif

        ngx_close_connection(pc);
        u->peer.connection = NULL;
    }

    ngx_argenta_stream_peer_connect(s);
    return;
}


void
ngx_argenta_stream_peer_finalize(ngx_argenta_stream_session_t *s, ngx_int_t rc)
{
    ngx_connection_t       *pc;
    ngx_stream_upstream_t  *u;

    ngx_log_debug1(NGX_LOG_DEBUG_STREAM, s->log, 0,
                   "finalize stream proxy: %i", rc);

    s->close = 1;
    u = s->upstream;
    pc = u->peer.connection;

    if (s->data) {//have init connection
        argenta_destory_connection(s);
    }
    else if (s->iconn && !s->manual){
        argenta_create_connection_fail(s->iconn);
    }

    if (u == NULL) {
        goto noupstream;
    }

    if (u->peer.free && u->peer.sockaddr) {
        u->peer.free(&u->peer, u->peer.data, 0);
        u->peer.sockaddr = NULL;
    }

noupstream:
    ngx_post_event(pc->read, &ngx_posted_events);
}

static void *
ngx_argenta_stream_peer_create_srv_conf(ngx_conf_t *cf)
{
    ngx_argenta_stream_peer_srv_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_argenta_stream_peer_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->ssl_protocols = 0;
     *     conf->ssl_ciphers = { 0, NULL };
     *     conf->ssl_name = { 0, NULL };
     *     conf->ssl_trusted_certificate = { 0, NULL };
     *     conf->ssl_crl = { 0, NULL };
     *     conf->ssl_certificate = { 0, NULL };
     *     conf->ssl_certificate_key = { 0, NULL };
     *
     *     conf->ssl = NULL;
     *     conf->upstream = NULL;
     */

    conf->buffer_size = NGX_CONF_UNSET_SIZE;

    conf->tries = NGX_CONF_UNSET_SIZE;
    conf->connect_timeout = NGX_CONF_UNSET_MSEC;

    conf->send_cache = NGX_CONF_UNSET_SIZE;
    conf->recv_cache = NGX_CONF_UNSET_SIZE;
    conf->send_chain_count = NGX_CONF_UNSET_SIZE;
    conf->local = NGX_CONF_UNSET_PTR;

#if (NGX_STREAM_SSL)
    conf->ssl_enable = NGX_CONF_UNSET;
    conf->ssl_session_reuse = NGX_CONF_UNSET;
    conf->ssl_server_name = NGX_CONF_UNSET;
    conf->ssl_verify = NGX_CONF_UNSET;
    conf->ssl_verify_depth = NGX_CONF_UNSET_UINT;
    conf->ssl_passwords = NGX_CONF_UNSET_PTR;
#endif

    return conf;
}


static char *
ngx_argenta_stream_peer_merge_srv_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_argenta_stream_peer_srv_conf_t *prev = parent;
    ngx_argenta_stream_peer_srv_conf_t *conf = child;

    ngx_conf_merge_size_value(conf->buffer_size,
                              prev->buffer_size, 16384);

    ngx_conf_merge_msec_value(conf->tries,
                              prev->tries, 1);

    ngx_conf_merge_msec_value(conf->connect_timeout,
                              prev->connect_timeout, 60000);

    ngx_conf_merge_msec_value(conf->send_cache,
                              prev->send_cache, 1500);

    ngx_conf_merge_size_value(conf->recv_cache,
                              prev->recv_cache, 1500);

    ngx_conf_merge_size_value(conf->send_chain_count,
                              prev->send_chain_count, 8);
    ngx_conf_merge_ptr_value(conf->local, prev->local, NULL);


#if (NGX_STREAM_SSL)

    ngx_conf_merge_value(conf->ssl_enable, prev->ssl_enable, 0);

    ngx_conf_merge_value(conf->ssl_session_reuse,
                              prev->ssl_session_reuse, 0);

    ngx_conf_merge_bitmask_value(conf->ssl_protocols, prev->ssl_protocols,
                              (NGX_CONF_BITMASK_SET|NGX_SSL_TLSv1
                               |NGX_SSL_TLSv1_1|NGX_SSL_TLSv1_2));

    ngx_conf_merge_str_value(conf->ssl_ciphers, prev->ssl_ciphers, "DEFAULT");

    ngx_conf_merge_str_value(conf->ssl_name, prev->ssl_name, "");

    ngx_conf_merge_value(conf->ssl_server_name, prev->ssl_server_name, 0);

    ngx_conf_merge_value(conf->ssl_verify, prev->ssl_verify, 0);

    ngx_conf_merge_uint_value(conf->ssl_verify_depth,
                              prev->ssl_verify_depth, 1);

    ngx_conf_merge_str_value(conf->ssl_trusted_certificate,
                              prev->ssl_trusted_certificate, "");

    ngx_conf_merge_str_value(conf->ssl_crl, prev->ssl_crl, "");

    ngx_conf_merge_str_value(conf->ssl_certificate,
                              prev->ssl_certificate, "");

    ngx_conf_merge_str_value(conf->ssl_certificate_key,
                              prev->ssl_certificate_key, "");

    ngx_conf_merge_ptr_value(conf->ssl_passwords, prev->ssl_passwords, NULL);

    if (conf->ssl_enable && ngx_argenta_stream_peer_set_ssl(cf, conf) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

#endif

    return NGX_CONF_OK;
}


#if (NGX_STREAM_SSL)

static ngx_int_t
ngx_argenta_stream_peer_set_ssl(ngx_conf_t *cf, ngx_argenta_stream_peer_srv_conf_t *pscf)
{
    ngx_pool_cleanup_t  *cln;

    pscf->ssl = ngx_pcalloc(cf->pool, sizeof(ngx_ssl_t));
    if (pscf->ssl == NULL) {
        return NGX_ERROR;
    }

    pscf->ssl->log = cf->log;

    if (ngx_ssl_create(pscf->ssl, pscf->ssl_protocols, NULL) != NGX_OK) {
        return NGX_ERROR;
    }

    cln = ngx_pool_cleanup_add(cf->pool, 0);
    if (cln == NULL) {
        return NGX_ERROR;
    }

    cln->handler = ngx_ssl_cleanup_ctx;
    cln->data = pscf->ssl;

    if (pscf->ssl_certificate.len) {

        if (pscf->ssl_certificate_key.len == 0) {
            ngx_log_error(NGX_LOG_EMERG, cf->log, 0,
                          "no \"argenta_ssl_certificate_key\" is defined "
                          "for certificate \"%V\"", &pscf->ssl_certificate);
            return NGX_ERROR;
        }

        if (ngx_ssl_certificate(cf, pscf->ssl, &pscf->ssl_certificate,
                                &pscf->ssl_certificate_key, pscf->ssl_passwords)
            != NGX_OK)
        {
            return NGX_ERROR;
        }
    }

    if (SSL_CTX_set_cipher_list(pscf->ssl->ctx,
                                (const char *) pscf->ssl_ciphers.data)
        == 0)
    {
        ngx_ssl_error(NGX_LOG_EMERG, cf->log, 0,
                      "SSL_CTX_set_cipher_list(\"%V\") failed",
                      &pscf->ssl_ciphers);
        return NGX_ERROR;
    }

    if (pscf->ssl_verify) {
        if (pscf->ssl_trusted_certificate.len == 0) {
            ngx_log_error(NGX_LOG_EMERG, cf->log, 0,
                      "no proxy_ssl_trusted_certificate for proxy_ssl_verify");
            return NGX_ERROR;
        }

        if (ngx_ssl_trusted_certificate(cf, pscf->ssl,
                                        &pscf->ssl_trusted_certificate,
                                        pscf->ssl_verify_depth)
            != NGX_OK)
        {
            return NGX_ERROR;
        }

        if (ngx_ssl_crl(cf, pscf->ssl, &pscf->ssl_crl) != NGX_OK) {
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}

#endif
/*
 *static char *
 *ngx_argenta_stream_peer_bind(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
 *{
 *    ngx_argenta_stream_peer_srv_conf_t *pscf = conf;
 *
 *    ngx_int_t   rc;
 *    ngx_str_t  *value;
 *
 *    if (pscf->local != NGX_CONF_UNSET_PTR) {
 *        return "is duplicate";
 *    }
 *
 *    value = cf->args->elts;
 *
 *    if (ngx_strcmp(value[1].data, "off") == 0) {
 *        pscf->local = NULL;
 *        return NGX_CONF_OK;
 *    }
 *
 *    pscf->local = ngx_palloc(cf->pool, sizeof(ngx_addr_t));
 *    if (pscf->local == NULL) {
 *        return NGX_CONF_ERROR;
 *    }
 *
 *    rc = ngx_parse_addr(cf->pool, pscf->local, value[1].data, value[1].len);
 *
 *    switch (rc) {
 *    case NGX_OK:
 *        pscf->local->name = value[1];
 *        return NGX_CONF_OK;
 *
 *    case NGX_DECLINED:
 *        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
 *                           "invalid address \"%V\"", &value[1]);
 *        [> fall through <]
 *
 *    default:
 *        return NGX_CONF_ERROR;
 *    }
 *}
 */
