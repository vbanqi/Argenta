#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>
#include <assert.h>

#include "ngx_argenta_api_module.h"

extern void argenta_app_core_api_process(ngx_http_request_t *r);
extern void argenta_app_core_api_cleanup(void *data);
extern void argenta_app_core_api_init(ngx_cycle_t *cycle);

static ngx_int_t ngx_argenta_api_init_process(ngx_cycle_t *cycle);
static char *ngx_argenta_api(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_argenta_api_postconfiguration(ngx_conf_t *cf);
static void * ngx_argenta_api_create_loc_conf(ngx_conf_t *cf);
static char * ngx_argenta_api_merge_loc_conf(ngx_conf_t *cf,
        void *parent, void *child);

void argenta_app_core_api_response(ngx_http_request_t *r, ngx_chain_t *ch);
ngx_int_t argenta_app_core_api_add_cleanup(ngx_http_request_t *r, void *data);

static time_t                       start_time;


/*
 * global: api-{bufs-{total,free,used}, total bytes in/out, bw in/out} - cscf
*/


typedef struct {
    ngx_uint_t                      api;
} ngx_argenta_api_loc_conf_t;

#define NGX_NSPR_STAT_ALL           0xff                                                               
#define NGX_NSPR_STAT_GLOBAL        0x01                                                               
#define NGX_NSPR_STAT_LIVE          0x02                                                               
#define NGX_NSPR_STAT_CLIENTS       0x04                                                               
#define NGX_NSPR_STAT_PLAY          0x08

static ngx_conf_bitmask_t           ngx_argenta_api_masks[] = {
    { ngx_string("all"),            NGX_NSPR_STAT_ALL           },
    { ngx_string("global"),         NGX_NSPR_STAT_GLOBAL        },
    { ngx_string("live"),           NGX_NSPR_STAT_LIVE          },
    { ngx_string("clients"),        NGX_NSPR_STAT_CLIENTS       },
    { ngx_null_string,              0 }
};


static ngx_command_t  ngx_argenta_api_commands[] = {

    { ngx_string("argenta_api"),
        NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
        ngx_argenta_api,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_argenta_api_loc_conf_t, api),
        ngx_argenta_api_masks },

    ngx_null_command
};


static ngx_http_module_t  ngx_argenta_api_module_ctx = {
    NULL,                               /* preconfiguration */
    ngx_argenta_api_postconfiguration,    /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    ngx_argenta_api_create_loc_conf,      /* create location configuration */
    ngx_argenta_api_merge_loc_conf,       /* merge location configuration */
};


ngx_module_t  ngx_argenta_api_module = {
    NGX_MODULE_V1,
    &ngx_argenta_api_module_ctx,          /* module context */
    ngx_argenta_api_commands,             /* module directives */
    NGX_HTTP_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    ngx_argenta_api_init_process,         /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_argenta_api_init_process(ngx_cycle_t *cycle)
{
    /*
     * HTTP process initializer is called
     * after event module initializer
     * so we can run posted events here
     */

    //ngx_event_process_posted(cycle, &ngx_argenta_init_queue);
    //argenta_app_core_api_init(cycle);
    
    //ngx_argenta_create_udp_connection(60001);

    return NGX_OK;
}

static ngx_int_t
ngx_argenta_api_handler(ngx_http_request_t *r)
{
    ngx_argenta_api_loc_conf_t       *slcf;
    //ngx_core_conf_t  *ccf;
    ngx_int_t           rc;

    /*
     *if (!(r->method & (NGX_HTTP_POST))) {
     *    return NGX_HTTP_NOT_ALLOWED;
     *}
     */

    /*
     *ccf = (ngx_core_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
     *                                       ngx_core_module);
     */
    //ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "process count %d", ccf->worker_processes);

    slcf = ngx_http_get_module_loc_conf(r, ngx_argenta_api_module);
    if (slcf->api == 0) {
        return NGX_DECLINED;
    }

    rc = ngx_http_read_client_request_body(r, argenta_app_core_api_process);
    if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
        return rc;
    }

    return NGX_DONE; // return done: will free connection(refrence count --)

/*
 *
 *    ch = ngx_alloc_chain_link(r->pool);
 *    if (ch == NULL) {
 *        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "alloc chain error");
 *        return NGX_HTTP_INTERNAL_SERVER_ERROR;
 *    }
 *
 *
 *    size_t len = 0;
 *    ngx_chain_t *pre = NULL;
 *    ngx_chain_t *cur = ch;
 *    while(cur) {
 *        len += cur->buf->last - cur->buf->pos;
 *        pre = cur;
 *        cur = cur->next;
 *    }
 *    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "----api result %z", len); 
 *    assert(len);
 *    
 *    pre->buf->last_buf = (r == r->main) ? 1 : 0;
 *    pre->buf->last_in_chain = 1;
 *    ngx_str_set(&r->headers_out.content_type, "application/json");
 *    ngx_http_send_header(r);
 *
 *    return ngx_http_output_filter(r, ch);
 */
}
ngx_int_t argenta_app_core_api_add_cleanup(ngx_http_request_t *r, void *data)
{

    ngx_http_cleanup_t            *cln;
    cln = ngx_http_cleanup_add(r, 0);
    if (cln == NULL) {
        return NGX_ERROR;
    }

    cln->handler = argenta_app_core_api_cleanup;
    cln->data = data;
    return NGX_OK;
}

void argenta_app_core_api_response(ngx_http_request_t *r, ngx_chain_t *ch)
{
    ngx_int_t rc;
    /*
     *r->headers_out.content_length_n = len;
     *r->headers_out.status = NGX_HTTP_OK;
     *ngx_str_set(&r->headers_out.content_type, "application/json");
     */
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR) {
        ngx_http_finalize_request(r, rc);
        return;
    }

    rc = ngx_http_output_filter(r, ch);
    ngx_http_finalize_request(r, rc);
}

static void *
ngx_argenta_api_create_loc_conf(ngx_conf_t *cf)
{
    ngx_argenta_api_loc_conf_t       *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_argenta_api_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->api = 0;

    return conf;
}


static char *
ngx_argenta_api_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_argenta_api_loc_conf_t       *prev = parent;
    ngx_argenta_api_loc_conf_t       *conf = child;

    ngx_conf_merge_bitmask_value(conf->api, prev->api, 0);

    return NGX_CONF_OK;
}


static char *
ngx_argenta_api(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_argenta_api_handler;

    return ngx_conf_set_bitmask_slot(cf, cmd, conf);
}


static ngx_int_t
ngx_argenta_api_postconfiguration(ngx_conf_t *cf)
{
    start_time = ngx_cached_time->sec;

    return NGX_OK;
}

