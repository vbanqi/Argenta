#include "Argenta.h"

#include "Config.h"


namespace argenta
{

Config::Config()
{
}

Config::~Config()
{
}

void Config::InitCtx(ngx_cycle_t *cycle)
{
    ngx_stream_conf_ctx_t *conf = (ngx_stream_conf_ctx_t *)cycle->conf_ctx[ngx_stream_module.index];
    ngx_stream_core_main_conf_t *main_conf = (ngx_stream_core_main_conf_t *)conf->main_conf[ngx_stream_core_module.ctx_index];
    ngx_stream_core_srv_conf_t **core_srv = (ngx_stream_core_srv_conf_t **)main_conf->servers.elts;

    ngx_argenta_stream_peer_srv_conf_t *peer_ctx = (ngx_argenta_stream_peer_srv_conf_t *)(*core_srv)->ctx->srv_conf[ngx_argenta_stream_peer_module.ctx_index];
    ngx_argenta_stream_srv_conf_t *stream_ctx = (ngx_argenta_stream_srv_conf_t *)(*core_srv)->ctx->srv_conf[ngx_argenta_stream_module.ctx_index];

    ngx_core_conf_t *ccf = (ngx_core_conf_t *) ngx_get_conf(cycle->conf_ctx, ngx_core_module);

    m_maxProcesses = ccf->worker_processes;
    m_sendCache = peer_ctx->send_cache;

    m_apiTimeOut = stream_ctx->api_timeout;
}
} // namespace argenta

