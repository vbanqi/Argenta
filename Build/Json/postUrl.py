##!/usr/bin/python
# -*- coding: UTF-8 -*-import urllib
import urllib2
import httplib

# url='http://10.2.250.207:1688';# req = urllib2.Request(url);
# print req;
# res_data = urllib2.urlopen(req);
# res = res_data.read();
# print res;

######################
#
######################

#host="192.168.1.104"
#host="10.2.250.207"
#host="10.1.13.44"
host="139.224.131.176"


body="m=1\n\
v=0\n\
br=2500000\n\
cid=1\n\
fr=30\n\
res=7\n\
sp=0\n\
dp=0\n\
ss=0\n\
pt=0\n\
a=0\n\
br=64000\n\
cid=0\n\
sr=48000\n\
sas=0\n\
cc=0\n\
sp=0\n\
dp=0\n\
ss=0\n\
pt=0\n"

headers={"Content-Type":"test/html","Content_Length":len(body)};

connection=httplib.HTTPConnection(host,1688);
connection.request("POST","/live/xyz/connect",body,headers);
response=connection.getresponse();
res_d = response.read();
print res_d[4:64]

body="sid=%s\n"%(res_d[4:64])
connection=httplib.HTTPConnection(host,1688);
connection.request("POST","/live/xyz/publish",body,headers);
response=connection.getresponse();
res_d1 = response.read();
print res_d1

body="sid=%s\n"%(res_d[4:64])
connection=httplib.HTTPConnection(host,1688);
connection.request("POST","/live/xyz/disconnect",body,headers);
response=connection.getresponse();
res_d2 = response.read();
print res_d2

exit()


body="{\"type\": \"connect\",\"data\": {\"mode\": 2\"v\": {\"bitrate\": 256000,\"codecID\": 3,\"frameRate\": 30,\"resolution\": 5,\"srcPort\": 1433,\"dstPort\": 1688,\"ssrc\": 1355756,\"payloadType\": 196},\
\"a\": {\"bitrate\": 640000,\"codecID\": 2,\"sampleRate\": 30,\"sampleSize\": 2048,\"channelCnt\": 1342,\"srcPort\": 1434,\"dstPort\": 1689,\"ssrc\": 1355756,\"payloadType\": 196}}}";
headers={"Content-Type":"test/html","Content_Length":len(body)};


connection=httplib.HTTPConnection("101.200.146.176",1688);
connection.request("POST","/live/xyz/connect",body,headers);

print "request!!"
response=connection.getresponse();

res_d = response.read();
print res_d;
resheader=response.getheaders();
print resheader;