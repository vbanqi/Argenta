##!/usr/bin/python
# -*- coding: UTF-8 -*-
import os
import sys
import json
import urllib
# from collections import OrderedDict  

if len(sys.argv) < 3:
	print "arguments error:the file name must be set up!"
	exit();

# print sys.argv;
# exit();

namespace = sys.argv[2];
structFile = sys.argv[3];
baseUri=""
if len(sys.argv) >4:
    baseUri = sys.argv[4];
if len(baseUri) == 0:
    baseUri = "./";

if not os.path.exists(baseUri):
    os.makedirs(baseUri)

try:
	file = open(sys.argv[1])
except IOError as e:
	print "11Error: con't find file %s"%(sys.argv[1])
	exit();

data=file.read();
file.close();
metadata=json.loads(data);

tab="    "

# print type(metadata);

def parseData(sp, name, data):
    ret = "%s%s %s;\n"%(sp, data, name);
    return ret;


def parseList(sp, name, data):
    ret = ""
    spa = sp + tab
    ret += "%sint %sCount;\n"%(sp, name);
    if type(data[0]) == dict:
        print data[0];
        ret += "%sstruct S%s{\n"%(sp, name);
        ret += parseObj(sp, name, data[0]);
        ret += "%s} *%s;\n"%(sp, name);
    elif type(data[0]) == list:
        ret += parseList(sp + tab, name, data[0]);
    else:
        ret += "%s%s*%s;\n"%(sp, data[0], name);

    return ret;

def parseObj(sp, name, data):
    ret = "";

    spa = sp + tab
    for i in data:
        if type(data[i]) == dict:
            ret += "%sstruct S%s{\n"%(spa, i);
            ret += parseObj(sp + tab, i, data[i]);
            ret += "%s} %s;\n"%(spa, i);
        elif type(data[i]) == list:
            ret += parseList(spa, i, data[i]);
        else:
            ret += parseData(spa, i, data[i]);

    return ret;



if __name__ == "__main__":
    try:
        file_str = open("%s%s.h"%(baseUri,structFile),"w")
    except IOError as e:
        print "Error: con't find file %s%s.h"%(baseUri,structFile)
        exit();

    file_str.write("#ifndef __%s_%s_h__\n#define __%s_%s_h__\n"%(namespace.upper(),structFile.upper(),namespace.upper(),structFile.upper()))
    cn=namespace[0] + namespace[1:]
    file_str.write("\nnamespace %s{\n"%(namespace))
    

    for i in metadata:
        if type(metadata[i]) == dict:
            ret = "%sstruct %s{\n"%(tab, i);
            ret += parseObj(tab, i, metadata[i])
            ret += "%s};\n"%(tab);
            file_str.write(ret)
            file_str.write("\n")

    file_str.write("}\n#endif")
    file_str.close()
    
    print "generate struct done"
