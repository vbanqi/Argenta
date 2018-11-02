#ifndef __ARGENTA_ERROR_TYPE_H_INCLUDED__
#define __ARGENTA_ERROR_TYPE_H_INCLUDED__


namespace argenta
{


enum ErrorType
{
    API_ERROR_NO_ERROR,
    API_ERROR_LOGIN_FAILED,
    API_ERROR_NOT_LOGIN,
    API_ERROR_NOT_SET,
    
    API_ERROR_INTER_ERROR,
    API_ERROR_INTERAPT,
    API_ERROR_PERMISSION_DENIED,
    API_ERROR_TIME_OUT
}; // class ErrorType

typedef struct {
    ErrorType type;
    const char *msg;
} ApiError;

static const ApiError g_ApiErrorEntity[] = {
	{API_ERROR_NO_ERROR, "Ok!"},
	{API_ERROR_LOGIN_FAILED, "Username or password error!"},
	{API_ERROR_NOT_LOGIN, "Please login before do this!"},
	{API_ERROR_NOT_SET, "Please set the user count for this server!"},
	{API_ERROR_INTER_ERROR, "Internal error!"},
	{API_ERROR_INTERAPT, "Connection was interapt!"},
	{API_ERROR_PERMISSION_DENIED, "Permission denied!"},
	{API_ERROR_TIME_OUT, "The request time out!"}
};

const int g_apiErrorEntityLength = sizeof(g_ApiErrorEntity) / sizeof(ApiError);

} // namespace argenta

#endif // ifndef __ARGENTA_ERROR_TYPE_H_INCLUDED__

