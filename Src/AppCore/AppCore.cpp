#include "Argenta.h"


#include "ApiDispatcher.h"
#include "Config.h"
#include "ArgentaLog.h"
//#include "SmartLog.h"
#include "IRequestCleanUp.h"
#include "NetInterface.h"
#include "NetConnection.h"

using namespace argenta;

extern "C" {
    ngx_int_t argenta_init_connection(ngx_argenta_stream_session_t *s);
    void argenta_destory_connection(ngx_argenta_stream_session_t *s);
    ngx_int_t argenta_start_connection(ngx_argenta_stream_session_t *s);
    void argenta_create_connection_fail(void *obj);

    void argenta_app_core_api_init(ngx_cycle_t *cycle);
    void argenta_app_core_api_cleanup(void *data);

    void argenta_app_core_api_process(ngx_http_request_t *r);

    void argenta_app_core_udp_send(ngx_event_t *wev);
    void argenta_app_core_udp_recv(ngx_event_t *rev);

    void argenta_app_core_tcp_send(ngx_event_t *wev);
    void argenta_app_core_tcp_recv(ngx_event_t *rev);
}

void argenta_app_core_api_init(ngx_cycle_t *cycle)
{
    Singleton<Config>::GetInstance()->InitCtx(cycle);
    //int maxProcesses = Singleton<Config>::GetInstance()->GetMaxProcesses();
    ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0,
            "argenta the session last process %d", ngx_last_process);
}

void argenta_app_core_api_process(ngx_http_request_t *r)
{
    ngx_int_t rc = Singleton<ApiDispatcher>::GetInstance()->OnRequest(r);
    if (rc < NGX_OK) {
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    }
    else if (rc > NGX_HTTP_SPECIAL_RESPONSE) {
        ngx_http_finalize_request(r, rc);
    }

}

void argenta_app_core_api_cleanup(void *data)
{
    IRequestCleanUp *cleanup = (IRequestCleanUp *)data;
    cleanup->OnCleanUp();
}

ngx_int_t argenta_init_connection(ngx_argenta_stream_session_t *s)
{
    ngx_connection_t *c = s->c;
    ngx_int_t   ret;

    ret = Singleton<NetInterface>::GetInstance()->CreateConnection(s); 
    ngx_log_error(NGX_LOG_INFO, c->log, 0,
            "argenta the session is opened:fd:%d, sess:%p, con:%p\n", s->c->fd, s, s->iconn);
    return ret;
}

void argenta_destory_connection(ngx_argenta_stream_session_t *s)
{
    ngx_log_error(NGX_LOG_INFO, s->log, 0,
            "argenta the session is closed:fd:%d, sess:%p, con:%p\n", s->c->fd, s, s->iconn);
    Singleton<NetInterface>::GetInstance()->OnDestoryConnection(s);
}

ngx_int_t argenta_start_connection(ngx_argenta_stream_session_t *s)
{
    return Singleton<NetInterface>::GetInstance()->ConnectionWillStart(s);
}

void argenta_create_connection_fail(void *obj)
{
    Singleton<NetInterface>::GetInstance()->CreateConnectionFail((IConnection *)obj);
}

void argenta_app_core_udp_send(ngx_event_t *wev)
{
    ngx_int_t                   ret;
    ssize_t                     n, sl;
    ngx_connection_t           *c, *uc; 
    ngx_argenta_stream_session_t  *s;
    ngx_buf_t                  *buf;
    NetConnection              *conn;

    uc = (ngx_connection_t *)wev->data;
    s = (ngx_argenta_stream_session_t *)uc->data;
    c = s->c;
    conn = (NetConnection *)s->data;

    if (c->destroyed) {
        ngx_log_error(NGX_LOG_INFO, c->log, 0,
                "argenta the udp connection is destoryed");
        return;
    }

    if (s->close) {
        ngx_stream_close_connection(uc);
        ngx_log_error(NGX_LOG_INFO, c->log, 0,
                "argenta the udp connection is closed");
        return;
    }


    while (wev->ready) {
        ret = conn->SendBuf(&buf);

        if (ret == NGX_DONE) {
            ngx_log_error(NGX_LOG_DEBUG, c->log, 0,
                    "argenta get send udp data done", ret);
            return;
        }

        sl = buf->last - buf->pos;
        n = c->send(c, buf->pos, sl);
        if (n == NGX_AGAIN) {
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "argenta send data not ok, will send on the event ready");
            break;
        }
        else if (n == NGX_ERROR) {
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "argenta send udp data error");
            return;
        }
        else if (n < sl) {
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "argenta could not send udp data at once");
            assert(false);
        }
        else if (n != sl) {
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "argenta could not send udp data at once");
            assert(false);
        }
        else {
            conn->SendDone(sl);
        }
    }
    if (ngx_handle_write_event(uc->write, 0) != NGX_OK) {
        ngx_argenta_stream_finalize(c, NGX_ERROR);
        ngx_log_error(NGX_LOG_ERR, c->log, 0,
                "argenta add event error");
        return;
    }
}

void argenta_app_core_udp_recv(ngx_event_t *rev)
{
    ngx_int_t                   ret, err;
    ngx_connection_t           *c, *uc; 
    NetConnection              *conn;
    ngx_argenta_stream_session_t  *s;
    ngx_buf_t                  *buf;
    

    uc = (ngx_connection_t *)rev->data;
    s = (ngx_argenta_stream_session_t *)uc->data;
    c = s->c;
    conn = (NetConnection *)s->data;
    buf = s->recv_buf;
    
    
    while(rev->ready) {
        ret = recvfrom(c->fd, buf->start, buf->end - buf->start, MSG_DONTWAIT, c->sockaddr, &c->socklen);
        if (ret == -1) {
            err = ngx_socket_errno;

            if (err == NGX_EAGAIN || err == NGX_EINTR) {
                ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                        "recvfrom() not ready");
                ret = NGX_AGAIN;

            } else {
                ret = ngx_connection_error(c, err, (char *)"recvfrom() failed");
            }
        }

        ngx_log_error(NGX_LOG_DEBUG, c->log, 0,
                "argenta udp connection recv: %d bytes", ret);
        if (ret == NGX_AGAIN) {
            break;
        }
        else if( ret == NGX_ERROR) {
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "udp recv error");
            //ngx_argenta_stream_finalize(c, NGX_ERROR);
            break;
        }
        buf->last = buf->start + ret;
        ret = conn->Recv(buf->start, buf->last - buf->start);
        if (ret != NGX_OK) {
            ngx_log_error(NGX_LOG_INFO, c->log, 0,
                    "argenta handler the udp connection error");
        }
    }
    
    if (s->udp_first) {
        ret = c->buffer->last - c->buffer->start;
        ngx_log_error(NGX_LOG_DEBUG, c->log, 0,
                "argenta udp connection recv: %d bytes", ret);
        ret = conn->Recv(c->buffer->start, ret);
        if (ret != NGX_OK) {
            ngx_log_error(NGX_LOG_INFO, c->log, 0,
                    "argenta handler the udp connection error");
        }
        s->udp_first = 0;
    }

    if (ngx_handle_read_event(uc->read, 0) != NGX_OK) {
        //ngx_argenta_stream_finalize(c, NGX_ERROR);
        ngx_log_error(NGX_LOG_ERR, c->log, 0,
                "argenta add event error");
        return;
    }

}

void argenta_app_core_tcp_send(ngx_event_t *wev)
{
    ngx_int_t          ret;
    ssize_t            n, sl;
    ngx_connection_t  *c;
    ngx_argenta_stream_session_t *s;
    ngx_buf_t          *buf;
    NetConnection      *mc;

    c = (ngx_connection_t *)wev->data;
    s = (ngx_argenta_stream_session_t *)c->data;

    if (c->destroyed) {
        ngx_log_error(NGX_LOG_INFO, c->log, 0,
                "argenta the tcp connection is destoryed");
        return;
    }

    if (s->close || wev->closed) {
        ngx_stream_close_connection(c);
        ngx_log_error(NGX_LOG_INFO, c->log, 0,
                "argenta the tcp connection is closed");
        return;
    }

    if (wev->timedout) {
        ngx_log_error(NGX_LOG_WARN, c->log, NGX_ETIMEDOUT, 
                "cycle: send: client timed out");
        c->timedout = 1;
        ngx_argenta_stream_finalize(c, NGX_ERROR);
        return;
    }

    mc = (NetConnection *)s->data;
    
    while (wev->ready) {
        ret = mc->SendBuf(&buf);
        if (ret == NGX_DONE) {
            ngx_log_error(NGX_LOG_DEBUG, c->log, 0,
                    "argenta send data done");
            return;
        }
        else if (ret < NGX_OK) {
            ngx_argenta_stream_finalize(c, NGX_ERROR);
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "argenta get send tcp data not ok");
            return;
        }
        sl = buf->last - buf->pos;
        n = c->send(c, buf->pos, sl);

        if (n == NGX_AGAIN) {
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "argenta send data again%d",n);
            break;
        }
        else if (n < 0) {
            ngx_argenta_stream_finalize(c, NGX_ERROR);
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "argenta recv tcp data error");
            return;
        }
        else if (n < sl) {
            mc->SendDone(n);
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "could not send data at once");
            break;
        }
        else if (n == sl) {
            mc->SendDone(n);
            ngx_log_error(NGX_LOG_DEBUG, c->log, 0,
                    "argenta send data %d done",n);
        }
        else{
            assert(false);
        }
    }

    if (ngx_handle_write_event(c->write, 0) != NGX_OK) {
        ngx_argenta_stream_finalize(c, NGX_ERROR);
        ngx_log_error(NGX_LOG_ERR, c->log, 0,
                "argenta add event error");
        return;
    }
}

void argenta_app_core_tcp_recv(ngx_event_t *rev)
{
    ngx_int_t          ret, len; 
    ssize_t            n;
    ngx_connection_t  *c;
    ngx_argenta_stream_session_t *s;
    ngx_buf_t          *buf;
    NetConnection      *mc;

    c = (ngx_connection_t *)rev->data;
    s = (ngx_argenta_stream_session_t *)c->data;

    if (c->destroyed) {
        ngx_log_error(NGX_LOG_INFO, c->log, 0,
                "argenta the tcp connection is destoryed fd:%d", c->fd);
        return;
    }

    if (s->close || rev->closed) {
        ngx_stream_close_connection(c);
        ngx_log_error(NGX_LOG_INFO, c->log, 0,
                "argenta the tcp connection is closed:fd:%d", c->fd);
        return;
    }
    mc = (NetConnection *)s->data;
    buf = s->recv_buf;
    assert(buf != NULL);

    while (rev->ready) {
        n = c->recv(c, buf->pos, buf->end - buf->pos);

        if (n == NGX_AGAIN) {
            break;
        }

        if (n == 0) {
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "argenta recv tcp close:fd:%d, err:%s", c->fd, strerror(errno));
            ngx_argenta_stream_finalize(c, NGX_ERROR);
            return;
        }
        else if (n == NGX_ERROR) {
            ngx_log_error(NGX_LOG_ERR, c->log, 0,
                    "argenta recv tcp error:%p", c);
            ngx_argenta_stream_finalize(c, NGX_ERROR);
            return;
        }

        /* process buf */
        buf->last = buf->pos + n;
        len = buf->last - buf->start;
        for (;;) {
            ret = mc->Recv(buf->start, len);

            if (ret == NGX_ERROR) {
                    ngx_log_error(NGX_LOG_ERR, c->log, 0,
                            "the msg process error");
                    ngx_argenta_stream_finalize(c, NGX_ERROR);
                    return;
            }
            else if (ret == NGX_AGAIN) {
                if (buf->last >= buf->end) {
                    ngx_log_error(NGX_LOG_ERR, c->log, 0,
                            "the msg is too long");
                    ngx_argenta_stream_finalize(c, NGX_ERROR);
                    return;
                }
                buf->pos = buf->last;
                break;
            }
            else if( ret == NGX_OK || ret == len) {
                buf->pos = buf->last = buf->start;
                break;
            }
            else {
                len -= ret;
                ngx_memmove(buf->start, buf->start + ret, len);
                buf->pos = buf->start + len;
                buf->last = buf->pos;
            }
        }
    }

    if (ngx_handle_read_event(rev, 0) != NGX_OK) {
        ngx_argenta_stream_finalize(c, NGX_ERROR);
        ngx_log_error(NGX_LOG_ERR, c->log, 0,
                "add event error");
    }
}
