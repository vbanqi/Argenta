#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream.h>
#include <ngx_argenta_stream.h>
#include <ngx_argenta.h>
#include <nginx.h>
#include <sys/param.h>
#include <fcntl.h>

extern void argenta_init_connection(ngx_argenta_stream_session_t *s);
extern void argenta_destory_connection(ngx_argenta_stream_session_t *s);
extern void argenta_create_connection_fail(void *obj);
extern void argenta_app_core_api_init(ngx_cycle_t *cycle);

extern void ngx_argenta_stream_peer_finalize(ngx_argenta_stream_session_t *s, ngx_int_t rc);

extern void argenta_app_core_tcp_recv(ngx_event_t *rev);
extern void argenta_app_core_tcp_send(ngx_event_t *wev);
extern void argenta_app_core_udp_recv(ngx_event_t *rev);
extern void argenta_app_core_udp_send(ngx_event_t *wev);

void ngx_argenta_create_udp_connection_server(const ngx_str_t *url, void *data);
void close_argenta_stream_tcp_listening();
void close_argenta_stream_udp_listening();

static void ngx_stream_argenta_handler(ngx_stream_session_t *s);
static char *ngx_stream_argenta_init(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static void * ngx_argenta_stream_create_srv_conf(ngx_conf_t *cf);
static char * ngx_argenta_stream_merge_srv_conf(ngx_conf_t *cf,
        void *parent, void *child);



ngx_argenta_stream_srv_conf_t          *argenta_srv_conf;
argenta_addrs_t                        *argenta_sockaddr_link;
/*
 * global: api-{bufs-{total,free,used}, total bytes in/out, bw in/out} - cscf
*/

static ngx_command_t  ngx_argenta_stream_commands[] = {

    { ngx_string("argenta_stream"),
        NGX_STREAM_SRV_CONF|NGX_CONF_NOARGS,
        ngx_stream_argenta_init,
        NGX_STREAM_SRV_CONF_OFFSET,
        0,
        NULL },
    { ngx_string("argenta_udp"),
        NGX_STREAM_SRV_CONF|NGX_CONF_NOARGS,
        ngx_stream_argenta_init,
        NGX_STREAM_SRV_CONF_OFFSET,
        0,
        NULL },
    { ngx_string("argenta_udp_pool_size"),
        NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_size_slot,
        NGX_STREAM_SRV_CONF_OFFSET,
        offsetof(ngx_argenta_stream_srv_conf_t, pool_size),
        NULL },
   
    { ngx_string("argenta_recv_cache"),
        NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_size_slot,
        NGX_STREAM_SRV_CONF_OFFSET,
        offsetof(ngx_argenta_stream_srv_conf_t, recv_cache),
        NULL},
    { ngx_string("argenta_send_cache"),
        NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_size_slot,
        NGX_STREAM_SRV_CONF_OFFSET,
        offsetof(ngx_argenta_stream_srv_conf_t, send_cache),
        NULL},
    { ngx_string("argenta_api_timeout"),
        NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_msec_slot,
        NGX_STREAM_SRV_CONF_OFFSET,
        offsetof(ngx_argenta_stream_srv_conf_t, api_timeout),
        NULL },
       
    ngx_null_command
};


static ngx_stream_module_t  ngx_argenta_stream_module_ctx = {
    NULL,    /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    ngx_argenta_stream_create_srv_conf,      /* create location configuration */
    ngx_argenta_stream_merge_srv_conf,       /* merge location configuration */
};


ngx_module_t  ngx_argenta_stream_module = {
    NGX_MODULE_V1,
    &ngx_argenta_stream_module_ctx,          /* module context */
    ngx_argenta_stream_commands,             /* module directives */
    NGX_STREAM_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    NULL,         /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    NGX_MODULE_V1_PADDING
};

static 
char *ngx_stream_argenta_init(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_stream_core_srv_conf_t  *cscf;

    cscf = ngx_stream_conf_get_module_srv_conf(cf, ngx_stream_core_module);

    cscf->handler = ngx_stream_argenta_handler;

    argenta_srv_conf = ngx_stream_conf_get_module_srv_conf(cf, ngx_argenta_stream_module);

    argenta_srv_conf->srv_conf = ((ngx_stream_conf_ctx_t *) cf->ctx)->main_conf;
    argenta_srv_conf->main_conf = ((ngx_stream_conf_ctx_t *) cf->ctx)->srv_conf;
    return NGX_CONF_OK;

}

static 
void ngx_stream_argenta_handler(ngx_stream_session_t *s)
{
    ngx_connection_t                *c, *uc;
    ngx_argenta_stream_srv_conf_t     *pscf;
    ngx_argenta_stream_session_t       *ss;

    c = s->connection;

    pscf = ngx_stream_get_module_srv_conf(s, ngx_argenta_stream_module);

    if (c->type == SOCK_DGRAM) {
        uc = c->listening->connection;
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0,
                "argenta udp connection not support");
        //ngx_stream_close_connection(c);
        /*
         *c->write->handler = argenta_app_core_udp_send;
         *c->read->handler = argenta_app_core_udp_recv;
         *c->read->handler(c->read);
         */
        //return;
    }
    else {
        uc = NULL;
    }

    ss = ngx_pcalloc(c->pool, sizeof(ngx_argenta_stream_session_t));
    if (!ss) {
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0,
                "argenta session alloc fail");
        ngx_stream_close_connection(c);
        return;
    }



    ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0,
                   "proxy connection handler");

    ss->pool = c->pool;
    ss->c = c;
    ss->type = pscf->type;
    ss->recv_buf = ngx_create_temp_buf(c->pool, pscf->recv_cache);
    if (!ss->recv_buf) {
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0,
                "argenta connection recv_buf alloc fail");
        ngx_stream_close_connection(c);
        return;
    }
    ss->send_buf = ngx_create_temp_buf(c->pool, pscf->send_cache);
    if (!ss->send_buf) {
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0,
                "argenta connection send_buf alloc fail");
        ngx_stream_close_connection(c);
        return;
    }

    ss->send_buf_size = pscf->send_cache;
    ss->recv_buf_size = pscf->recv_cache;

    ss->role = SERVER;
    ss->log = c->log;
    ss->data = s;

    ss->sockaddr = c->sockaddr;
    ss->socklen = c->socklen;
    ss->peer_name = &c->addr_text;

    ss->local_sockaddr = c->local_sockaddr;
    ss->local_socklen = c->local_socklen;


    if (c->type == SOCK_DGRAM) {
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0,
                "proxy connection ready udp");
        ss->uc = uc;
        ss->udp_first = 1;
        uc->data = ss;
        uc->write->handler = argenta_app_core_udp_send;
        uc->read->handler = argenta_app_core_udp_recv;
        ngx_post_event(uc->read, &ngx_posted_events);
    }
    else {
        c->data = ss;
        c->write->handler = argenta_app_core_tcp_send;
        c->read->handler = argenta_app_core_tcp_recv;
    }

    argenta_init_connection(ss);
    
    if (c->type == SOCK_STREAM) {
        if (c->read->ready) {
            ngx_post_event(c->read, &ngx_posted_events);
        }
        if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
            ngx_argenta_stream_finalize(c, NGX_ERROR);
            return;
        }
    }

}
void close_argenta_stream_tcp_listening()
{
    //ngx_stream_conf_ctx_t *conf = ngx_cycle->conf_ctx[ngx_stream_module.index];
    ngx_stream_core_main_conf_t *main_conf = ngx_stream_get_module_main_conf(argenta_srv_conf, ngx_stream_core_module);
    
    ngx_uint_t         i;
    ngx_listening_t   *ls;
    ngx_connection_t  *c;

    if (ngx_event_flags & NGX_USE_IOCP_EVENT) {
        return;
    }

    ngx_accept_mutex_held = 0;
    ngx_use_accept_mutex = 0;

    ls = main_conf->listen.elts;
    for (i = 0; i < main_conf->listen.nelts; i++) {

        if (ls[i].sockaddr->sa_family == SOCK_DGRAM) {
            continue;
        }
        c = ls[i].connection;

        if (c) {
            if (c->read->active) {
                if (ngx_event_flags & NGX_USE_EPOLL_EVENT) {

                    /*
                     * it seems that Linux-2.6.x OpenVZ sends events
                     * for closed shared listening sockets unless
                     * the events was explicitly deleted
                     */

                    ngx_del_event(c->read, NGX_READ_EVENT, 0);

                } else {
                    ngx_del_event(c->read, NGX_READ_EVENT, NGX_CLOSE_EVENT);
                }
            }

            ngx_free_connection(c);

            c->fd = (ngx_socket_t) -1;
        }

        ls[i].connection = NULL;

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                       "close listening %V #%d ", &ls[i].addr_text, ls[i].fd);

        if (ngx_close_socket(ls[i].fd) == -1) {
            ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, ngx_socket_errno,
                          ngx_close_socket_n " %V failed", &ls[i].addr_text);
        }

#if (NGX_HAVE_UNIX_DOMAIN)

        if (ls[i].sockaddr->sa_family == AF_UNIX
            && ngx_process <= NGX_PROCESS_MASTER
            && ngx_new_binary == 0)
        {
            u_char *name = ls[i].addr_text.data + sizeof("unix:") - 1;

            if (ngx_delete_file(name) == NGX_FILE_ERROR) {
                ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, ngx_socket_errno,
                              ngx_delete_file_n " %s failed", name);
            }
        }

#endif

        ls[i].fd = (ngx_socket_t) -1;
    }

}
void close_argenta_stream_udp_listening()
{
    ngx_stream_core_main_conf_t *main_conf = ngx_stream_get_module_main_conf(argenta_srv_conf, ngx_stream_core_module);
    
    ngx_uint_t         i;
    ngx_listening_t   *ls;
    ngx_connection_t  *c;

    if (ngx_event_flags & NGX_USE_IOCP_EVENT) {
        return;
    }

    ngx_accept_mutex_held = 0;
    ngx_use_accept_mutex = 0;

    ls = main_conf->listen.elts;
    for (i = 0; i < main_conf->listen.nelts; i++) {

        if (ls[i].sockaddr->sa_family == SOCK_STREAM) {
            continue;
        }
        c = ls[i].connection;

        if (c) {
            if (c->read->active) {
                if (ngx_event_flags & NGX_USE_EPOLL_EVENT) {

                    /*
                     * it seems that Linux-2.6.x OpenVZ sends events
                     * for closed shared listening sockets unless
                     * the events was explicitly deleted
                     */

                    ngx_del_event(c->read, NGX_READ_EVENT, 0);

                } else {
                    ngx_del_event(c->read, NGX_READ_EVENT, NGX_CLOSE_EVENT);
                }
            }

            ngx_free_connection(c);

            c->fd = (ngx_socket_t) -1;
        }

        ls[i].connection = NULL;

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                       "close listening %V #%d ", &ls[i].addr_text, ls[i].fd);

        if (ngx_close_socket(ls[i].fd) == -1) {
            ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, ngx_socket_errno,
                          ngx_close_socket_n " %V failed", &ls[i].addr_text);
        }

#if (NGX_HAVE_UNIX_DOMAIN)

        if (ls[i].sockaddr->sa_family == AF_UNIX
            && ngx_process <= NGX_PROCESS_MASTER
            && ngx_new_binary == 0)
        {
            u_char *name = ls[i].addr_text.data + sizeof("unix:") - 1;

            if (ngx_delete_file(name) == NGX_FILE_ERROR) {
                ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, ngx_socket_errno,
                              ngx_delete_file_n " %s failed", name);
            }
        }

#endif

        ls[i].fd = (ngx_socket_t) -1;
    }

}

static void *
ngx_argenta_stream_create_srv_conf(ngx_conf_t *cf)
{
    ngx_argenta_stream_srv_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_argenta_stream_srv_conf_t));
    if (conf == NULL) {
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, cf->log, 0,
                "argenta conf alloc fail");
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

    conf->api_timeout = NGX_CONF_UNSET_MSEC;
    conf->pool_size = NGX_CONF_UNSET_SIZE;
    conf->send_cache = NGX_CONF_UNSET_SIZE;
    conf->recv_cache = NGX_CONF_UNSET_SIZE;
    conf->send_chain_count = NGX_CONF_UNSET_SIZE;
    return conf;
}


static char *
ngx_argenta_stream_merge_srv_conf(ngx_conf_t *cf, void *parent, void *child)
{
    (void)cf;
    ngx_argenta_stream_srv_conf_t *prev = parent;
    ngx_argenta_stream_srv_conf_t *conf = child;

    ngx_conf_merge_msec_value(conf->api_timeout,
                              prev->api_timeout, 5000);

    ngx_conf_merge_size_value(conf->pool_size,
                              prev->pool_size, 16 * 1024);
    ngx_conf_merge_size_value(conf->send_cache,
                              prev->send_cache, 2 * 1024);
    ngx_conf_merge_size_value(conf->recv_cache,
                              prev->recv_cache, 2 * 1024);
    ngx_conf_merge_size_value(conf->send_chain_count,
                              prev->send_chain_count, 1024);

    return NGX_CONF_OK;
}

void
ngx_argenta_stream_finalize(ngx_connection_t *c, ngx_int_t rc)
{
    ngx_argenta_stream_session_t *s;

    s = c->data;

    if (s->close) {
        return;
    }

    if (s->upstream) {
        ngx_argenta_stream_peer_finalize(s, rc);
        return;
    }

    argenta_destory_connection(s);

    ngx_log_debug1(NGX_LOG_DEBUG_STREAM, s->c->log, 0,
                   "finalize stream proxy: %i", rc);

    s->close = 1;

    //ngx_stream_close_connection(c);
    ngx_post_event(c->read, &ngx_posted_events);
}


