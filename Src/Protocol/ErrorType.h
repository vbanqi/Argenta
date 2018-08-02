#ifndef __ARGENTA_ERROR_TYPE_H_INCLUDED__
#define __ARGENTA_ERROR_TYPE_H_INCLUDED__


namespace argenta
{


enum ErrorType
{
    API_ERROR_NO_ERROR,
    API_ERROR_STREAM_ERROR,
    API_ERROR_PUSH_ERROR,
    API_ERROR_STREAM_PUSH_ERROR,
    API_ERROR_RECORD_ERROR,
    API_ERROR_RECORD_STREAM_ERROR,
    API_ERROR_RECORD_PUSH_ERROR,
    API_ERROR_RECORD_PUSH_STREAM_ERROR,

    API_ERROR_ID_INVALID,
    API_ERROR_CONNECT_FAIL,
    API_ERROR_RESOURCE_LOWER,
    API_ERROR_RELAY_RESOURCE_LOWER,
    API_ERROR_PARSER_ERROR,
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
	{API_ERROR_STREAM_ERROR, "The stream create error!"},
	{API_ERROR_PUSH_ERROR, "push to rtmp error!"},
	{API_ERROR_STREAM_PUSH_ERROR, "The stream pull and rtmp push error!"},
	{API_ERROR_RECORD_ERROR, "The record error!"},
	{API_ERROR_RECORD_STREAM_ERROR, "The record and stream create error!"},
	{API_ERROR_RECORD_PUSH_ERROR, "The record and rtmp create error!"},
	{API_ERROR_RECORD_PUSH_STREAM_ERROR, "The record, rtmp create and stream create error!"},
	{API_ERROR_ID_INVALID, "The live id not exist!"},
	{API_ERROR_CONNECT_FAIL, "Connect relay fail!"},
	{API_ERROR_RESOURCE_LOWER, "The resources is low!"},
	{API_ERROR_RELAY_RESOURCE_LOWER, "The relay resources is low!"},
	{API_ERROR_PARSER_ERROR, "Parser error!"},
	{API_ERROR_INTER_ERROR, "Internal error!"},
	{API_ERROR_INTERAPT, "Connection was interapt!"},
	{API_ERROR_PERMISSION_DENIED, "Permission denied!"},
	{API_ERROR_TIME_OUT, "The request time out!"}
};

const int g_apiErrorEntityLength = sizeof(g_ApiErrorEntity) / sizeof(ApiError);

} // namespace argenta

#endif // ifndef __ARGENTA_ERROR_TYPE_H_INCLUDED__

