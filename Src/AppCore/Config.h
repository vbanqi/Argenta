#ifndef __ARGENTA_CONFIG_H_INCLUDED__
#define __ARGENTA_CONFIG_H_INCLUDED__

#include "ngx_argenta_stream.h"

#define SConf Singleton<Config>::GetInstance()

namespace argenta
{

class Config
{
public:
	Config();
	virtual ~Config();

    void InitCtx(ngx_cycle_t *cycle);
    

    ngx_msec_t GetApiTimeOut() { return m_apiTimeOut; }

    int GetMaxProcesses() { return m_maxProcesses; }
    int GetMaxVirtualPort() { return m_maxPortCount; }

private:
    ngx_str_t *m_myInternalHost;
    ngx_str_t *m_myHost;
    ngx_str_t *m_schHost;
    ngx_str_t *m_ccode;
    ngx_str_t *m_pcode;
    ngx_str_t *m_icode;
    size_t m_capability;
    size_t m_sessionCacheSize;
    size_t m_sendCache;
    ngx_msec_t m_schHeatbeat;
    int m_maxProcesses;
    int m_maxPortCount;
    int m_relayPort;
    int m_localPort;
    int m_startPort;

    ngx_msec_t m_apiTimeOut;
    ngx_msec_t m_maxKeepalived;
    ngx_msec_t m_maxVideoKeyGap;
    ngx_msec_t m_sessTimeOut;
    ngx_msec_t m_sendMccTimer;
    ngx_msec_t m_sendRrTimer;
    ngx_msec_t m_retryTimer;
    ngx_msec_t m_audioLossTimer;
    ngx_msec_t m_videoLossTimer;
    size_t m_audioJitter;
    size_t m_videoJitter;

    int m_mediaThreadSize;

    ngx_str_t *m_logLevel;
    ngx_str_t *m_nginxLogPath;
    ngx_str_t *m_argentaLogPath;
    int m_logSize;
    int m_logBackup;
}; // class Config

} // namespace argenta

#endif // ifndef __ARGENTA_CONFIG_H_INCLUDED__

