#ifndef __ARGENTA_ARGENTA_OBJ_H_INCLUDED__
#define __ARGENTA_ARGENTA_OBJ_H_INCLUDED__

namespace argenta
{

#define ArgentaSLog(level, format, args...) ArgentaLog::LogMessage(0, level, __FILE__, __FUNCTION__, __LINE__, "%V|" format, GetLogIdentify(), ##args)

class ArgentaObj
{
public:
	ArgentaObj();
	virtual ~ArgentaObj();

    void SetLogIdentify(ngx_str_t *dt) { m_identify = *dt; }

    ngx_str_t *GetLogIdentify() { return &m_identify; }

private:
    ngx_str_t m_identify;
}; // class ArgentaObj

} // namespace argenta

#endif // ifndef __ARGENTA_ARGENTA_OBJ_H_INCLUDED__

