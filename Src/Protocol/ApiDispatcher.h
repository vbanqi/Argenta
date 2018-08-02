#ifndef __ARGENTA_API_DISPATCHER_H_INCLUDED__
#define __ARGENTA_API_DISPATCHER_H_INCLUDED__

#include "ErrorType.h"

#define SApi Singleton<ApiDispatcher>::GetInstance()
namespace argenta
{

class SchParser;
class MemPool;
class ApiDispatcher
{
public:
	ApiDispatcher();
	virtual ~ApiDispatcher();

    ngx_int_t OnRequest(ngx_http_request_t *r);

private:
    enum REUQUEST_TYPE{
        REQUEST_DEFAULT = 0, 
    };

    ngx_int_t OnHandler(ngx_http_request_t *r, REUQUEST_TYPE type);

private:
    static const off_t kMaxBodyLength = 4096;
    static const size_t kDefaultChunkSize = 4 * 1024;
}; // class ApiDispatcher

} // namespace argenta

#endif // ifndef __ARGENTA_API_DISPATCHER_H_INCLUDED__

