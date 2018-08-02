#ifndef __ARGENTA_LOG_H_INCLUDED__ 
#define __ARGENTA_LOG_H_INCLUDED__ 

namespace argenta 
{

/*
 *#define ARGENTA_LOG_DEBUG 100
 *#define ARGENTA_LOG_INFO 101
 *#define ARGENTA_LOG_WARN 102
 *#define ARGENTA_LOG_ERROR 103
 */

#define ARGENTA_LOG_STDERR            0
#define ARGENTA_LOG_EMERG             1
#define ARGENTA_LOG_ALERT             2
#define ARGENTA_LOG_CRIT              3
#define ARGENTA_LOG_ERR               4
#define ARGENTA_LOG_WARN              5
#define ARGENTA_LOG_NOTICE            6
#define ARGENTA_LOG_INFO              7
#define ARGENTA_LOG_DEBUG             8

#define NGINX       LogInfo::LOG_MODULE_NGINX
#define LOG         LogInfo::LOG_MODULE_LOG
#define COMMON      LogInfo::LOG_MODULE_COMMON
#define EDGE        LogInfo::LOG_MODULE_EDGE
#define PROTOCOL    LogInfo::LOG_MODULE_PROTOCOL
#define MEDIA       LogInfo::LOG_MODULE_MEDIA
#define STASTISTICS LogInfo::LOG_MODULE_STATISTICS
#define SESSION     LogInfo::LOG_MODULE_SESSION   

#define ArgentaLog(loglevel, format, args...) ngx_argenta_log_write(loglevel, __FILE__, __FUNCTION__, __LINE__, format, ##args);

//ArgentaLog::LogMessage(loglevel, __FILE__, __FUNCTION__, __LINE__, format, ##args)
#define ArgentaDebug(format, args...) ArgentaLog(ARGENTA_LOG_DEBUG, format, ##args)
#define ArgentaInfo(format, args...) ArgentaLog(ARGENTA_LOG_INFO, format, ##args)
#define ArgentaWarn(format, args...) ArgentaLog(ARGENTA_LOG_WARN, format, ##args)
#define ArgentaError(format, args...) ArgentaLog(ARGENTA_LOG_ERR, format, ##args)
#define ArgentaStream(format, args...) //ArgentaLog(ARGENTA_LOG_ERROR, format, ##args)

} // namespace argenta 

#endif // ifndef __ARGENTA_LOG_H_INCLUDED__ 

