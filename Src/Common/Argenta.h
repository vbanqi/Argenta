#ifndef __ARGENTA_ARGENTA_H__
#define __ARGENTA_ARGENTA_H__

#ifndef NGINX_OFF
#ifdef __cplusplus
extern "C" {
#endif
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <nginx.h>
#include <ngx_http.h>
#include <ngx_argenta.h>
#ifdef __cplusplus
}
#endif
#else
typedef unsigned char u_char;
#endif
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <string>
#include <memory>
#include <map>

#include "ArgentaLog.h"

#include "Singleton.h"

#define nullptr 0

#define ArgentaAssert(s) assert(s)


typedef std::shared_ptr<ngx_str_t> StrPtr;

inline uint64_t SystemTimesMillis() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000; 
}


#endif // __ARGENTA_ARGENTA_H__

