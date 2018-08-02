//#include "MetadataParser.h"
#include "ProtocolData.h"
#include "ProtocolRecordParser.h"

using namespace hola;

//static Metadata playMetadata = {
//   IC_MODE_PLAY,
//   {
//       18888,//int ssrc;
//       1,//int payloadType;    
//       16000,//int sampleSize;
//       441000,//int sampleRate;
//       2,//int channelCnt;
//       50002//int port;
//   },
//   {
//       264,//int codecID;
//       166666,//int ssrc;
//       512000,//int bitrate;
//       25,//int frameRate;
//       0,//int payloadType;
//       IC_RES_VGA,//ICResulution resolution;
//       50000//int port;
//   }
//};

const char* data_str = "{"
        "\"liveId\": \"xyz\","
        "\"confId\": \"confid\","
        "\"appId\": \"appid\","
        "\"userName\": \"user\","
        "\"userId\": \"123456\","
        "\"consumeCapability\":100,"
        "\"relayIp\": \"10.1.1.3\","
        "\"rtmp\": ["
            "\"10.1.12.13\","
            "\"10.1.12.14\","
            "\"10.1.12.15\""
        "],"
        "\"storage\": {"
            "\"enable\":1,"
            "\"vendorType\":\"oss\""
        "}"
    "}";

int main()
{
//MetadataParser *parser = new MetadataParser();
//parser->parseData(&playMetadata);
//char* str;
//int buffersize;
//parser->GetJson(&str,buffersize);
//printf("%s\n", str);
//// parser->parseFirst(sst,strlen(sst));

    ProtocolRecordParser *parser = new ProtocolRecordParser();
    ProtocolRecord *data = nullptr;
    parser->ParseJson(data_str, strlen(data_str), &data);
    printf("%s\n",data->rtmp[0]);
    return 0;
}
