##!/usr/bin/python
# -*- coding: UTF-8 -*-
import os
import sys
import json
import urllib

if len(sys.argv) < 2:
    print "arguments error:the file name must be set up!"
    exit();

rapidJsonPath=sys.argv[2]
namespace=sys.argv[3]
baseDataName=sys.argv[4]
if len(sys.argv) > 5:
    baseUri=sys.argv[5]
if len(baseUri) == 0:
    baseUri = "./"
if not os.path.exists(baseUri):
    os.makedirs(baseUri)


tab="    "

def printAddDataMember(space, name, data, meta, isFromList=False):
    ret = ""
    if data == "int":
        ret +="%srapidjson::Value %s(rapidjson::kNumberType);\n"%(space, name);
        if isFromList:
            ret +="%s%s.SetInt(%s);\n"%(space, name, meta);
        else:
            ret +="%s%s.SetInt(%s%s);\n"%(space, name, meta, name);
    elif data == "const char*" or data == "char*":
        ret +="%srapidjson::Value %s(rapidjson::kStringType);\n"%(space, name);
        if isFromList:
            ret +="%s%s.SetString(%s, *m_allocator);\n"%(space, name, meta);
        else:
            ret +="%s%s.SetString(%s%s, *m_allocator);\n"%(space, name, meta, name);
    elif data == "bool":
        ret +="%srapidjson::Value %s(rapidjson::kTrueType);\n"%(space, name);
        if isFromList:
            ret +="%s%s.SetBool(%s);\n"%(space, name, meta);
        else:
            ret +="%s%s.SetBool(%s%s);\n"%(space, name, meta, name);
    else:
        ret +="%srapidjson::Value %s(rapidjson::kNumberType);\n"%(space, name);
        if isFromList:
            ret +="%s%s.SetInt((int)%s);\n"%(space, name, meta);
        else:
            ret +="%s%s.SetInt((int)%s%s);\n"%(space, name, meta, name);
    return ret;

def printAddArrayMember(space, name, data, meta):
    # s="%s[i]."%(sd);
    ret = ""
    spa = space + tab
    if type(data[0]) == dict:
        ret += "%srapidjson::Value array(rapidjson::kArrayType);\n"%(spa);
        m = "%s%s[i]."%(meta, name);
        ret += printAddObjectMember(spa, "array", data[0], m);
        ret += "%s%s.PushBack(array, *m_allocator);\n"%(spa, name);
    elif type(data[0]) == list:
        m = "%s%s[i]."%(meta, name);
        ret += printAddArrayMember(spa, "array", data[0], m);
        ret +="%s%s.PushBack(array, *m_allocator);\n"%(spa, name);
    else:
        m = "%s%s[i]"%(meta, name);
        ret += printAddDataMember(spa, "array", data[0], m, True);
        ret +="%s%s.PushBack(array, *m_allocator);\n"%(spa, name);
    return ret;

def printAddObjectMember(space, name, data, meta):
    ret =""
    for i in data:
        if type(data[i]) == dict:
            spa = space + tab;
            s="%s%s."%(meta, i);
            ret += "%srapidjson::Value %s(rapidjson::kObjectType);\n"%(space, i);
            ret += "%s{\n"%(space);
            ret += printAddObjectMember(spa, i, data[i], s);
            ret += "%s}\n"%(space)
        elif type(data[i]) == list:
            ret += "%srapidjson::Value %s(rapidjson::kArrayType);\n"%(space, i);
            ret +="%sfor(int i = 0; i < %s%sCount; i++) {\n"%(space, meta, i);
            ret += printAddArrayMember(space, i, data[i], meta)
            ret +="%s}\n"%(space);
        else:
            ret += printAddDataMember(space, i, data[i], meta)
        ret += "%s%s.AddMember(\"%s\", %s, *m_allocator);\n"%(space, name, i, i);
    return ret;

def printOutDataMember(space, name, data, doc, meta, isFromList=False):
    global baseDataName;
    ret = ""
    spa = space + tab;
    if data == "int":
        if isFromList:
            ret +="%sif (%s.IsInt()) {\n"%(space, doc);
            ret +="%s%s = %s.GetInt();\n"%(spa, meta, doc);
        else:
            ret +="%sif (%s[\"%s\"].IsInt()) {\n"%(space, doc, name);
            ret +="%s%s%s = %s[\"%s\"].GetInt();\n"%(spa, meta, name, doc, name);
        ret +="%s}\n"%(space);

    elif data == "const char*":
        if isFromList:
            ret +="%sif (%s.IsString()) {\n"%(space, doc);
            ret +="%s%s = %s.GetString();\n"%(spa, meta, doc);
        else:
            ret +="%sif (%s[\"%s\"].IsString()) {\n"%(space, doc, name);
            ret +="%s%s%s = %s[\"%s\"].GetString();\n"%(spa, meta, name, doc, name);

        ret +="%s}\n"%(space);
    elif data == "char*":
        if isFromList:
            ret +="%sif (%s.IsString()) {\n"%(space, doc);
            ret +="%s%s = (%s)%s.GetString();\n"%(spa, meta, data, doc);
        else:
            ret +="%sif (%s[\"%s\"].IsString()) {\n"%(space, doc, name);
            ret +="%s%s%s = (%s)%s[\"%s\"].GetString();\n"%(spa, meta, name, data, doc, name);

        ret +="%s}\n"%(space);
    elif data == "bool":
        if isFromList:
            ret +="%sif (%s.IsBool()) {\n"%(space, doc);
            ret +="%s%s = %s.GetBool();\n"%(spa, meta, data, doc);
        else:
            ret +="%sif (%s[\"%s\"].IsBool()) {\n"%(space, doc, name);
            ret +="%s%s%s = %s[\"%s\"].GetBool();\n"%(spa, meta, name, doc, name);

        ret +="%s}\n"%(space);
    else:
        if isFromList:
            ret +="%sif (%s.IsInt()) {\n"%(space, doc);
            ret +="%s%s = %s.GetInt();\n"%(spa, meta, doc);
        else:
            ret +="%sif (%s[\"%s\"].IsInt()) {\n"%(space, doc, name);
            ret +="%s%s%s = (%s)%s[\"%s\"].GetInt();\n"%(spa, meta, name, data, doc, name);
        ret +="%s}\n"%(space);
    return ret;

def printOutArrayMember(space, name, data, doc, meta, classname):
    #key:meta->
    ret = ""
    spa = space + tab;
    if type(data[0]) == dict:
        metai = "%s[i]."%(meta)
        doci  = "%s[i]"%(doc)
        #skt: document["channels"][i].
        #sk:  meta->channels[i]
        classname +="::S%s"%(name)
        ret += printOutMember(spa, name, data[0], doci, metai, classname)
    elif type(data[0]) == list:
        metai="%s[i]."%(meta);
        doci="%s[i]"%(doc)
        # skt: document["channels"]
        # sk:  meta->channels
        # d[i]:channels
        ret += printOutArrayMember(spa, name, data[0], doci, metai, classname);
    else:
        metai="%s[i]"%(meta);
        doci="%s[i]"%(doc)
        ret += printOutDataMember(spa, name, data[0], doci, metai, True)

    return ret;


def printOutMember(space, name, data, doc, meta, classname):
    ret = "";
    spa = space + tab;
    for i in data:
        ret +="%sif(%s.HasMember(\"%s\")) {\n"%(space, doc, i);
        if type(data[i]) == dict:
            ret +="%sif (%s[\"%s\"].IsObject()) {\n"%(spa, doc, i);
            metai="%s%s."%(meta,i);
            doci="%s[\"%s\"]"%(doc,i)
            classname +="::S%s"%(i)
            ret += printOutMember(spa + tab, i, data[i], doci, metai, classname);
            ret +="%s}\n"%(spa);
        elif type(data[i]) == list:
            ret +="%sif (%s[\"%s\"].IsArray()) {\n"%(spa, doc, i);
            metai="%s%s"%(meta, i);
            doci="%s[\"%s\"]"%(doc, i)
            # skt: document["channels"]
            # sk:  meta->channels
            # d[i]:channels
            ret +="%sint s = %s[\"%s\"].Size();\n"%(spa + tab, doc, i);
            ret +="%s%s%sCount = s;\n"%(spa + tab, meta, i);
            classname +="::S%s"%(i)
            if data[i][0] == "const char*" or data[i][0] == "char*" or data[i][0] == "int":
                ret +="%s%s%s = (%s*)m_allocator->Malloc(sizeof(%s) * s);\n"%(spa + tab, meta, i, data[i][0], data[i][0]);
            else:#
                ret +="%s%s%s = (%s*)m_allocator->Malloc(sizeof(%s%s) * s);\n"%(spa + tab, meta, i, classname, meta, i);

            ret +="%sfor(int i = 0; i < s; i++) {\n"%(spa + tab);
            ret += printOutArrayMember(spa + tab, i, data[i], doci, metai, classname);
            ret +="%s}\n"%(spa + tab);
            ret +="%s}\n"%(spa);
        else:
            ret += printOutDataMember(spa, i, data[i], doc, meta)
        ret +="%s}\n"%(space);
    return ret;

def printH(file_h,classname,key,value):
    global rapidJsonPath;
    global namespace;
    ret ="#ifndef __%s_%s_H__\n#define __%s_%s_H__\n"%(namespace.upper(),classname.upper(),namespace.upper(),classname.upper());
    ret +="#include \"%s/rapidjson.h\"\n"%(rapidJsonPath);
    ret +="#include \"%s/document.h\"\n"%(rapidJsonPath);
    ret +="#include \"%s/stringbuffer.h\"\n"%(rapidJsonPath);
    ret +="#include \"%s/writer.h\"\n"%(rapidJsonPath);
    if baseDataName == "":
        ret +="#include \"%s.h\"\n"%(key);
    else:
        ret +="#include \"%s.h\"\n"%(baseDataName);

    ret +="namespace %s{\n\n"%(namespace);
    ret +="struct %s;\n"%(key);
    ret +="template <typename Alloc = rapidjson::MemoryPoolAllocator<>>\n";
    ret +="class %s\n{\npublic:\n    %s();\n   ~%s();\n"%(classname,classname,classname);
    ret +="    typedef rapidjson::GenericDocument<rapidjson::UTF8<> , Alloc> DocumentCustom;\n";
    ret +="\n";
    ret +="    void SetAllocator(Alloc *allocator = nullptr);\n";
    ret +="    bool ParseData(%s* meta, char *buffer, size_t len, size_t &outlen);\n"%(key);
    ret +="    bool ParseJson(const char* data, int size, %s** meta);\n\n"%(key);
    ret +="    void Release();\n";
    ret +="\n";
    ret +="private:\n";
    ret +="    const static int m_maxContentLen = 4096;\n";
    ret +="    char m_buffer[m_maxContentLen];\n";
    ret +="    Alloc *m_allocator;\n";
    ret +="    Alloc *m_ownAllocator;\n";
    ret +="};\n";
    ret +="#include \"%s.inl\"\n"%(classname);
    ret +="} //namespace %s\n"%(namespace);
    ret +="#endif";
    return ret;

def printCPP(file_cpp,classname,key,value):
    global rapidJsonPath;
    global namespace;
    global baseDataName;
    ret ="";

    ret +="\n\n";
    ret +="template<typename Alloc>\n";
    ret +="%s<Alloc>::%s()\n"%(classname,classname);
    ret +="    : m_allocator(nullptr)\n";
    ret +="    , m_ownAllocator(nullptr)\n";
    ret +="{\n";
    ret +="\n";
    ret +="}\n";
    ret +="\n";
    ret +="template<typename Alloc>\n";
    ret +="%s<Alloc>::~%s()\n"%(classname,classname);
    ret +="{\n";
    ret +="    Release();\n";
    ret +="}\n";
    ret +="template<typename Alloc>\n";
    ret +="void %s<Alloc>::SetAllocator(Alloc *allocator)\n"%(classname);
    ret +="{\n";
    ret +="    m_allocator = allocator;\n";
    ret +="}\n";
    ret +="\n";
    ret +="template<typename Alloc>\n";
    ret +="bool %s<Alloc>::ParseData(%s* meta, char *buffer, size_t len, size_t &outlen)\n"%(classname,key);
    ret +="{\n";
    ret +="    if (m_allocator == nullptr) {\n";
    ret +="        m_ownAllocator = m_allocator = new Alloc();\n";
    ret +="    }\n";
    ret +="    DocumentCustom document(m_allocator);\n";
    ret +="\n";
    ret +="    rapidjson::Value data(rapidjson::kObjectType);\n";
    ret += printAddObjectMember(tab, "data", value, "meta->");

    ret +="\n";
    ret +="    rapidjson::StringBuffer buff;\n";
    ret +="\n";
    ret +="    rapidjson::Writer<rapidjson::StringBuffer> writer(buff);\n";
    ret +="    data.Accept(writer);\n";
    ret +="    const char* buf = buff.GetString();\n";
    ret +="    outlen = buff.GetSize();\n";
    ret +="    if (outlen >= len) {\n";
    ret +="        //error \n";
    ret +="        return false;\n";
    ret +="    }\n";
    ret +="    memcpy(buffer, buf, outlen);\n";
    ret +="    buffer[outlen] = '\\0';\n";
    ret +="    return true;\n";
    ret +="}\n";
    ret +="\n";
    ret +="template<typename Alloc>\n";
    ret +="bool %s<Alloc>::ParseJson(const char* data, int size, %s** meta)\n"%(classname,key);
    ret +="{\n";
    ret +="    memcpy(m_buffer, data, size);\n";
    ret +="    m_buffer[size] = '\\0';\n";
    ret +="    if (m_allocator == nullptr) {\n";
    ret +="        m_ownAllocator = m_allocator = new Alloc();\n";
    ret +="    }\n";
    ret +="    DocumentCustom document(m_allocator);\n";
    ret +="    \n";
    ret +="    document.Parse(m_buffer);\n";
    ret +="    if (document.HasParseError()) {\n";
    ret +="        return false;\n";
    ret +="    }\n";
    ret +="    *meta = (%s *)m_allocator->Malloc(sizeof(%s));\n"%(key, key);
    ret +="    memset(*meta, 0, sizeof(%s));\n"%(key);
    ret +="\n";
    ret += printOutMember(tab, "data", value, "document", "(*meta)->", key)
    ret +="    return true;\n";
    ret +="}\n";
    ret +="template<typename Alloc>\n";
    ret +="void %s<Alloc>::Release()\n"%(classname);
    ret +="{\n";
    ret +="    delete m_ownAllocator;\n";
    ret +="    m_ownAllocator = nullptr;\n";
    ret +="    m_allocator = nullptr;\n";
    ret +="}\n";
    ret +="\n";
    return ret;

def passData(key,value):
    name_h="%sParser"%(key);
    name_cpp="%sParser"%(key);

try:
    file = open(sys.argv[1])
except IOError as e:
    print "Error: con't find file %s"%(sys.argv[1])
    exit();

data=file.read();
file.close();

metadata=json.loads(data);

for i in metadata:
    classname = "%sParser"%(i);
    name_h="%s%s.h"%(baseUri,classname);
    try:
        file_h = open(name_h,"w");
    except IOError as e:
        print "Error: con't find file %s"%(name_h)
        exit();
    ret = printH(tab, classname,i,metadata[i]);
    file_h.write(ret);
    file_h.close();
    name_cpp="%s%s.inl"%(baseUri,classname);
    try:
        file_cpp = open(name_cpp,"w");
    except IOError as e:
        print "Error: con't find file %s"%(name_cpp)
        exit();
    ret = printCPP(tab, classname,i,metadata[i]);
    file_cpp.write(ret);
    file_cpp.close();

print "generate parser done"
