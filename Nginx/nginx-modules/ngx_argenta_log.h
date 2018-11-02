#ifndef _NGX_ARGENTA_LOG_H_INCLUDED_
#define _NGX_ARGENTA_LOG_H_INCLUDED_

typedef struct ngx_argenta_log_conf_s ngx_argenta_log_conf_t;
typedef struct argenta_log_node_s argenta_log_node_t;
typedef struct argenta_log_s argenta_log_t;

struct argenta_log_s {
    ngx_str_t               name;
    ngx_str_t               fullname;

    ngx_uint_t              level;
    size_t                  size;
    ngx_uint_t              backup;
    size_t                  count;
    ngx_open_file_t        *file;
    ngx_queue_t             files;

    ngx_msec_t              check_timer;
    ngx_msec_t              cache_time;
    u_char                  cache_str[128];
};

struct ngx_argenta_log_conf_s {
    argenta_log_t              *argenta_log;
    argenta_log_t              *argenta_ngx_log;

    ngx_cycle_t             *cycle;
    ngx_log_t               *log;

    ngx_int_t                roll;

};


struct argenta_log_node_s {
    u_char              filename[MAXNAMLEN];
    ngx_queue_t         queue;
};


extern void ngx_argenta_log_write(ngx_uint_t level, const char *file, 
            const char *func, int line, const char *format, ...);

extern ngx_int_t ngx_argenta_log_write_data(u_char *data, size_t len);
#endif
