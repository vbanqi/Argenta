#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_argenta_log.h>
#include <nginx.h>
#include <ngx_stream.h>
#include <sys/param.h>
#include <fcntl.h>
#include <assert.h>

static char *ngx_argenta_log_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_argenta_ngx_log_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_argenta_block(ngx_conf_t *cf, argenta_log_t *lg);

static ngx_int_t init_argenta_log(ngx_cycle_t *cycle);

static ngx_int_t ngx_argenta_queue_filename_cmp(const ngx_queue_t *a, const ngx_queue_t *b);
static ngx_int_t ngx_argenta_get_all_log_file(ngx_conf_t *cf, ngx_queue_t *q, u_char *path, ngx_str_t *name);

void ngx_argenta_log_write(ngx_uint_t level, const char *file, 
            const char *func, int line, const char *format, ...);
ngx_int_t ngx_argenta_log_write_data(u_char *data, size_t len);

static void ngx_argenta_roll_event(ngx_event_t *ev);

static ngx_int_t ngx_argenta_roll_file(argenta_log_t *lg);

//static void ngx_argenta_ngx_log_writer(ngx_log_t *log, ngx_uint_t level, u_char *buf, size_t len);

static void * ngx_argenta_log_create_main_conf(ngx_conf_t *cf);
static char * ngx_argenta_log_init_main_conf(ngx_conf_t *cf, void *conf);


static const ngx_msec_t         log_roll_timer = 3 * 1000;

static ngx_event_t             *log_roll_event = NULL;
ngx_argenta_log_conf_t            *argenta_log_conf;

/*
 * global: api-{bufs-{total,free,used}, total bytes in/out, bw in/out} - cscf
*/

static ngx_str_t err_levels[] = {
    ngx_null_string,
    ngx_string("emerg"),
    ngx_string("alert"),
    ngx_string("crit"),
    ngx_string("error"),
    ngx_string("warn"),
    ngx_string("notice"),
    ngx_string("info"),
    ngx_string("debug")
};

static ngx_command_t  ngx_argenta_log_commands[] = {

    { ngx_string("argenta_log"),
      NGX_STREAM_MAIN_CONF|NGX_CONF_TAKE1234,
      ngx_argenta_log_block,
      0,
      0,
      NULL },
    { ngx_string("argenta_ngx_log"),
      NGX_STREAM_MAIN_CONF|NGX_CONF_TAKE12,
      ngx_argenta_ngx_log_block,
      0,
      0,
      NULL },
    
    ngx_null_command
};


static ngx_stream_module_t ngx_argenta_log_module_ctx = {
    NULL,                                  /* postconfiguration */

    ngx_argenta_log_create_main_conf,      /* create main configuration */
    ngx_argenta_log_init_main_conf,        /* init main configuration */

    NULL,       /* create server configuration */
    NULL         /* merge server configuration */

};


ngx_module_t  ngx_argenta_log_module = {
    NGX_MODULE_V1,
    &ngx_argenta_log_module_ctx,          /* module context */
    ngx_argenta_log_commands,             /* module directives */
    NGX_STREAM_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    init_argenta_log,         /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_int_t init_argenta_log(ngx_cycle_t *cycle)
{
    argenta_log_conf->roll = 1;
    log_roll_event = NULL;
    if (ngx_process_slot != 0) {
        return NGX_OK;
    }

    log_roll_event = ngx_pcalloc(cycle->pool, sizeof(ngx_event_t));
    log_roll_event->log = cycle->log;
    log_roll_event->data = cycle;
    log_roll_event->handler = ngx_argenta_roll_event;

    if (!log_roll_event->timer_set)
        ngx_add_timer(log_roll_event, log_roll_timer);

    return NGX_OK;
}

static char *ngx_argenta_log_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    argenta_log_t                 *lg;
    ngx_argenta_log_conf_t        *pmcf;

    pmcf = ngx_stream_conf_get_module_main_conf(cf, ngx_argenta_log_module);
    pmcf->cycle = cf->cycle;
    pmcf->log = cf->log;

    lg = pmcf->argenta_log;
    
    return ngx_argenta_block(cf, lg);
}

static char *ngx_argenta_ngx_log_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_argenta_log_conf_t    *pmcf = *(void **)conf;
    argenta_log_t                  *lg;
    //char                       *ret;
    ngx_str_t                  *value;
    ngx_str_t                  *ngxln, name;
    ngx_uint_t                  argsn;
    size_t                     *sp;
    u_char                     *pd, *fp;

    pmcf = ngx_stream_conf_get_module_main_conf(cf, ngx_argenta_log_module);
    pmcf->cycle = cf->cycle;
    pmcf->log = cf->log;

    lg = pmcf->argenta_ngx_log;

    argsn = cf->args->nelts;

    value = cf->args->elts;

    if (argsn > 1) {

        sp = &lg->size;
        if (*sp != NGX_CONF_UNSET_SIZE) {
            return "is duplicate";
        }

        *sp = ngx_parse_size(&value[1]);
        if (*sp == (size_t) NGX_ERROR) {
            return "invalid value";
        }
    }
    
    if (argsn > 2) {
        if (lg->backup != NGX_CONF_UNSET_UINT) {
            return "is duplicate";
        }

        int bk = ngx_atoi(value[2].data, value[2].len);
        if (bk == NGX_ERROR) {
            return "log backup error";
        }
        lg->backup = bk;
    }

    ngxln = &cf->cycle->new_log.file->name;

    if (ngxln->len < sizeof(NGX_PREFIX)) {
        return "nginx error log is not set";
    }

    fp = ngxln->data;

    pd = (u_char *)strrchr((char *)fp, '/');
    *pd = '\0';
    name.data = pd + 1;
    name.len = ngxln->len - (pd - fp) - 1;

    lg->count = ngx_argenta_get_all_log_file(cf, &lg->files, fp, &name);
    *pd = '/';

    lg->name.data = ngxln->data + sizeof(NGX_PREFIX) - 1;
    lg->name.len = ngxln->len - sizeof(NGX_PREFIX) + 1;

    lg->fullname = *ngxln;

    lg->file = cf->cycle->new_log.file;
    lg->level = cf->cycle->new_log.log_level;

    return NGX_CONF_OK;
}

static char *ngx_argenta_block(ngx_conf_t *cf, argenta_log_t *lg)
{
    ngx_str_t                  *value;
    ngx_uint_t                  argsn, n;
    ngx_int_t                   ret, found = 0;
    size_t                     *sp;
    char                        fdp[MAXPATHLEN];
    char                        filepath[MAXPATHLEN] = {0};
    u_char                     *pd;


    argsn = cf->args->nelts;

    value = cf->args->elts;

    lg->name = value[1];

    if (argsn > 2) {
        for (n = 1; n <= NGX_LOG_DEBUG; n++) {
            if (ngx_strcmp(value[2].data, err_levels[n].data) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            return "log level error";
        }
        lg->level = n;
    }

    if (argsn > 3) {

        sp = &lg->size;
        if (*sp != NGX_CONF_UNSET_SIZE) {
            return "is duplicate";
        }

        *sp = ngx_parse_size(&value[3]);
        if (*sp == (size_t) NGX_ERROR) {
            return "invalid value";
        }
    }
    
    if (argsn > 4) {
        if (lg->backup != NGX_CONF_UNSET_UINT) {
            return "is duplicate";
        }

        int bk = ngx_atoi(value[4].data, value[4].len);
        if (bk == NGX_ERROR) {
            return "log backup error";
        }
        lg->backup = bk;
    }

    if (snprintf(fdp, MAXPATHLEN, "/proc/self/fd/%d", cf->log->file->fd) == -1) {
        return "get file path error";
    }
    if ((ret = readlink(fdp, filepath, MAXPATHLEN)) == -1) {
        return "get file path error";
    }
    
    pd = (u_char *)strrchr(filepath, '/');
    *pd = '\0';
    lg->count = ngx_argenta_get_all_log_file(cf, &lg->files, (u_char *)&filepath[0], &lg->name);

    lg->file = ngx_conf_open_file(cf->cycle, &lg->name);
    if (lg->file == NULL) {
        return "open log error";
    }

    lg->fullname = lg->name;
    if (ngx_conf_full_name(argenta_log_conf->cycle, &lg->fullname, 0) != NGX_OK) {
        return "get full name fail";
    }
    return NGX_CONF_OK;
}

static ngx_int_t ngx_argenta_queue_filename_cmp(const ngx_queue_t *a, const ngx_queue_t *b)
{
    argenta_log_node_t *hla = ngx_queue_data(a, argenta_log_node_t, queue);
    argenta_log_node_t *hlb = ngx_queue_data(b, argenta_log_node_t, queue);
    return ngx_strcmp(hla->filename, hlb->filename);
}

static ngx_int_t ngx_argenta_get_all_log_file(ngx_conf_t *cf, ngx_queue_t *q, u_char *path, ngx_str_t *name)
{
    DIR* dir;
    struct dirent *ptr;
    u_char *p;
    ngx_int_t ret = 0;
    ngx_str_t fn;
    ngx_queue_t     *que;

    fn.data = (u_char *)strrchr((const char *)name->data, '/');
    if (fn.data == NULL) {
        fn.data = name->data;
    }
    else {
        fn.data++;
    }
    p = (u_char*)ngx_strnstr(name->data, ".log", name->len);
    fn.len = p - fn.data;

    if ((dir = opendir((const char *)path)) == NULL) {
        return NGX_ERROR;
    }

    while ((ptr = readdir(dir)) != NULL) {
        if (ptr->d_type == DT_REG) {
            p = (u_char*)strstr(ptr->d_name, ".log");
            if (!p) {
                continue;
            }

            if (ngx_strncmp(ptr->d_name, fn.data, fn.len) == 0 
                    && ngx_strncmp(ptr->d_name, fn.data, fn.len + sizeof(".log")) != 0) {
                
                argenta_log_node_t *hl = ngx_calloc(sizeof(argenta_log_node_t), cf->log);
                if (hl == NULL) {
                    closedir(dir);
                    return NGX_ERROR;
                }
                p = ngx_cpymem(hl->filename, (u_char *)path, ngx_strlen(path));
                *p++ = '/';
                ngx_memcpy(p, (u_char *)ptr->d_name, ngx_strlen(ptr->d_name));

                que = &hl->queue;
                ngx_queue_insert_tail(q, que);
                ret++;
            }
        }
    }

    ngx_queue_sort(q, ngx_argenta_queue_filename_cmp);
    
    closedir(dir);
    return ret;
}


void ngx_argenta_log_write(ngx_uint_t level, const char *file, 
            const char *func, int line, const char *format, ...)
{
    va_list      args;
    u_char      *p, *last;
    u_char       errstr[NGX_MAX_ERROR_STR];
    ngx_str_t    err;
    argenta_log_t  *lg;
    
    lg = argenta_log_conf->argenta_log;

    if (level > lg->level) {
        return;
    }

    err.data = errstr;
    last = errstr + NGX_MAX_ERROR_STR;

    if (lg->cache_time != ngx_current_msec) {
        int msec = ngx_current_msec % 1000;
        time_t sec = ngx_current_msec / 1000 + 8 * 3600;
        ngx_tm_t tm;
        ngx_gmtime(sec, &tm);

        p = ngx_snprintf(lg->cache_str, NGX_MAX_ERROR_STR, "%04d/%02d/%02d %02d:%02d:%02d.%03d" 
                "|%P"
                , tm.tm_year, tm.tm_mon, tm.tm_mday
                , tm.tm_hour, tm.tm_min, tm.tm_sec, msec
                , ngx_log_pid);
        *p = 0;
        lg->cache_time = ngx_current_msec;
    }
    const u_char *liveId = (u_char *)"argenta";
    p = ngx_snprintf(errstr, sizeof(errstr), "%s"
            "|%V|%s|%s:%d|"
            , lg->cache_str
            , &err_levels[level]
            , liveId
            , file, line);
    //ngx_linefeed(p);

    va_start(args, format);
    p = ngx_vslprintf(p, last, format, args);
    va_end(args);

    if (p > last - NGX_LINEFEED_SIZE) {
        p = last - NGX_LINEFEED_SIZE;
    }

    ngx_linefeed(p);
    err.len = p - errstr;

    ngx_argenta_log_write_data(err.data, err.len);
}

ngx_int_t ngx_argenta_log_write_data(u_char *data, size_t len)
{
    ssize_t              n;
    argenta_log_t          *lg;

    lg = argenta_log_conf->argenta_log;

    if (lg->file->fd == NGX_INVALID_FILE) {
        ngx_log_stderr(0,  "the log fd is invalid!");
        return NGX_ERROR;
    }

    n = ngx_write_fd(lg->file->fd, data, len);

    if (n == -1)
    {
        ngx_log_stderr(0,  "the log fd:%d write failed:%s", lg->file->fd, strerror(errno));
        return NGX_ERROR;
    }
    
    return NGX_OK;
}

static void ngx_argenta_roll_event(ngx_event_t *ev)
{
    ngx_argenta_roll_file(argenta_log_conf->argenta_ngx_log);
    ngx_argenta_roll_file(argenta_log_conf->argenta_log);
    if (!ev->timer_set)
        ngx_add_timer(ev, log_roll_timer);
}

static ngx_int_t ngx_argenta_roll_file(argenta_log_t *lg)
{
    ngx_int_t       ret;
    ngx_str_t       lname;
    u_char          newname[MAXPATHLEN];
    u_char          oldname[MAXPATHLEN];
    u_char         *end;
    argenta_log_node_t     *hl;
    ngx_queue_t     *q;
    ngx_file_info_t     finfo;
    ssize_t             size;

    time_t          sec;
    ngx_tm_t        tm;

    lname = lg->fullname;

    if (ngx_fd_info(lg->file->fd, &finfo) == NGX_FILE_ERROR) {
        ngx_log_stderr(0,  "the log get fd:%d info failed!", lg->file->fd);
        return NGX_ERROR;
    }

    size = ngx_file_size(&finfo);
    if (size < (ssize_t)lg->size && argenta_log_conf->roll) {
        return NGX_OK;
    }

    end = ngx_cpymem(oldname, lname.data, lname.len);
    *end = 0;

    lname.data = oldname;
    lname.len = end - oldname - (sizeof(".log") - 1);

    sec = ngx_current_msec / 1000 + 8 * 3600;
    ngx_gmtime(sec, &tm);

	end = ngx_snprintf(newname, MAXPATHLEN, "%V_%04d%02d%02d%02d%02d%02d.log" 
                , &lname, tm.tm_year, tm.tm_mon, tm.tm_mday
                , tm.tm_hour, tm.tm_min, tm.tm_sec);
    *end = 0;

    if (ngx_rename_file(oldname, newname) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,  "the log rename %s to %s failed!", oldname, newname);
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,  "the log name %V fullname %V failed!", &lg->name, &lg->fullname);
        return NGX_ERROR;
    }

    hl = ngx_calloc(sizeof(argenta_log_node_t), ngx_cycle->log);
    if (hl == NULL) {
        ngx_log_stderr(0,  "ngx_calloc argenta_log_node_t failed!");
        return NGX_ERROR;
    }
    end = ngx_cpymem(hl->filename, (u_char *)newname, end - newname);
    *end = 0;

    ngx_queue_insert_tail(&lg->files, &hl->queue);
    lg->count++;
    q = ngx_queue_head(&lg->files);
    while (lg->count > lg->backup) {
        hl = ngx_queue_data(q, argenta_log_node_t, queue);
        ret = ngx_delete_file(hl->filename);
        if (ret == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ERR, argenta_log_conf->log, 0,
                    "argenta delete log fail:%s", hl->filename);
            break;
        }
        q = q->next;
        ngx_queue_remove(&hl->queue);
        ngx_free(hl);
        lg->count--;
    }

    return ngx_os_signal_process(argenta_log_conf->cycle, (char *)"reopen", getppid());
}

static void * 
ngx_argenta_log_create_main_conf(ngx_conf_t *cf)
{
    ngx_argenta_log_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_argenta_log_conf_t));
    if (conf == NULL) {
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, cf->log, 0,
                "argenta conf alloc fail");
        return NULL;
    }
    conf->cycle = cf->cycle;
    conf->log = cf->log;

    conf->argenta_log = ngx_pcalloc(cf->pool, sizeof(argenta_log_t));
    if (conf->argenta_log == NULL) {
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, cf->log, 0,
                "argenta conf argenta log alloc fail");
        return NULL;
    }
    conf->argenta_ngx_log = ngx_pcalloc(cf->pool, sizeof(argenta_log_t));
    if (conf->argenta_ngx_log == NULL) {
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, cf->log, 0,
                "argenta conf argenta ngx log alloc fail");
        return NULL;
    }

    argenta_log_conf = conf;
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
    ngx_queue_init(&conf->argenta_log->files);
    ngx_queue_init(&conf->argenta_ngx_log->files);

    ngx_str_t initname = ngx_string("argenta.log");
    conf->argenta_log->name = initname;
    conf->argenta_log->level = NGX_CONF_UNSET_UINT;
    conf->argenta_log->size = NGX_CONF_UNSET;
    conf->argenta_log->backup = NGX_CONF_UNSET_UINT;
    ngx_str_t initngxname = ngx_string("nginx.log");
    conf->argenta_ngx_log->name = initngxname;
    conf->argenta_ngx_log->level = NGX_CONF_UNSET_UINT;
    conf->argenta_ngx_log->size = NGX_CONF_UNSET;
    conf->argenta_ngx_log->backup = NGX_CONF_UNSET_UINT;
    return conf;
}


static char * 
ngx_argenta_log_init_main_conf(ngx_conf_t *cf, void *cof)
{
    ngx_argenta_log_conf_t *conf = cof;

    ngx_conf_init_uint_value(conf->argenta_ngx_log->level, 0);

    ngx_conf_init_size_value(conf->argenta_ngx_log->size, 30 * 1024 * 1024);

    ngx_conf_init_uint_value(conf->argenta_ngx_log->backup, 10);

    ngx_conf_init_uint_value(conf->argenta_log->level, 0);

    ngx_conf_init_size_value(conf->argenta_log->size, 30 * 1024 * 1024);

    ngx_conf_init_uint_value(conf->argenta_log->backup, 10);

    return NGX_CONF_OK;
}

