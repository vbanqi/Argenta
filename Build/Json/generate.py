##!/usr/bin/python
# -*- coding: UTF-8 -*-
import os
import sys
import json

# print sys.argv[0]

if len(sys.argv) < 2:
	print "arguments error:the file name must be set up!"
	exit()
if 	len(sys.argv) == 3:
	cahceFile = sys.argv[2];
else :
	cahceFile = "cache.json"


NAMESPACE="";
ENUMS=[];
CLASS="";
data={};

STACK=[];
con=False;
i=0;
def parseStruct():
	global i;
	start=False;
	structname="";
	data={}
	while i<len(STACK):
		if STACK[i] == "{":
			start=True;
			i+=1;
		if STACK[i].startswith("}"):
			# print data;
			return data;
		if start:
			key=STACK[i];
			if key == "struct":
				i+=1;
				if STACK[i]=="{":
					value = parseStruct();
					if ";" not in STACK[i]:
						i+=1;
						key = STACK[i].split(";")[0];
				else :
					key = STACK[i];
				data[key]=value;
			else :
				i+=1;
				value=STACK[i].split(";")[0];
				data[value]=key;

			i+=1;
	return data;




def parseStack():
	global NAMESPACE;
	global STACK;
	global ENUMS;
	global i;
	data={};
	while i<len(STACK):
		if STACK[i]=="namespace":
			i+=1;
			NAMESPACE=STACK[i];
			i+=1;
		elif STACK[i] == "enum":
			i+=1;
			ENUMS+=STACK[i];
			i+=1;
		elif STACK[i]=="struct":
			i+=1;
			if STACK[i]=="{":
				data+=parseStruct();
			else:
				key = STACK[i];
				i+=1;
				value = parseStruct();
				data[key]= value;

		i+=1;
	return data;


def parseLine(l):
	global STACK;

	data=l.split("//");
	ls=data[0].rstrip();
	STACK+=ls.split();
	return;


def filterLine(l):
	if len(l)>0:
		l.lstrip();
		if l.startswith('#') or line=='\r\n':
			return;
		else:
			l.replace("\n","");
			l.replace("\r","");
			parseLine(l);
	return;

def writeFile(name,data):
	try:
		fw = open(name,"w");
	except IOError as e:
		print "Error: con't find file %s"%(sys.argv[1])
		exit();
	# print data;
	fw.write(data);
	fw.close();
	return;

try:
	file = open(sys.argv[1])
except IOError as e:
	print "Error: con't find file %s"%(sys.argv[1])
	exit()

# file = open(file_name,"r")
line = file.readline();
while len(line) >0:
	filterLine(line);
	line = file.readline()

dt = parseStack();

file.close();
# print NAMESPACE;
# print ENUMS;
# print dt;
path = os.getcwd()+"/%s"%(cahceFile);
encode_json = json.dumps(dt);
# print encode_json;
# print type(dt)
writeFile(path,encode_json);
#输出解析类
