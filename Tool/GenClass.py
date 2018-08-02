#!/usr/bin/python

import sys
import getopt
import os

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg;

g_fileName = ""
g_namespace = "argenta"
g_classList = []
g_funcList = []
g_inheritList = []
def AddFile(fileName):
    print "add file: " + fileName
    g_fileName = fileName

def AddClass(className):
    print "add class: " + className
    g_classList.append(className)

def AddFunc(funcName):
    print "add function: " + funcName
    if g_funcList:
        print g_funcList
    g_funcList.append(funcName)

def AddInherit(inheritName):
    print "add inherit: " + inheritName
    g_inheritList.append(inheritName)

def WriteHeaderFunction(headerStringList, classname):
    for fn in g_funcList:
        headerStringList.append("\t" + fn + ";\n")
        
def WriteCppFunction(cppStringList, classname):
    for fn in g_funcList:
        fnList = fn.split(' ', 1)
        if fnList[0] == "virtual":
            fnList = fnList[1].split(' ', 1)

        returnValue = 0 # void
        if fnList[1][0] == '*':
            cppStringList.append(fnList[0] +" *" + classname + "::" + fnList[1][1:] + "\n")
            returnValue = 1 # ptr
        else:
            cppStringList.append(fnList[0] +" " + classname + "::" + fnList[1] + "\n")
            if (fnList[0] == "int"):
                returnValue = 2
            elif (fnList[0] == "bool"):
                returnValue = 3
        cppStringList.append("{" + "\n")
        if (returnValue == 1):
            cppStringList.append("\treturn nullptr;" + "\n");
        elif (returnValue == 2):
            cppStringList.append("\treturn " + g_namespace.upper() + "_ERROR;" + "\n");
        elif (returnValue == 3):
            cppStringList.append("\treturn false;" + "\n");
        cppStringList.append("}" + "\n")
        cppStringList.append("" + "\n")

def GenerateClass():
    if not g_classList:
        print "class name should be set"
        return 0

    fileName = g_classList[0]
    if g_fileName:
        fileName = g_fileName

    if os.path.exists(fileName + ".cpp"):
        print "file: " + fileName + ".cpp exist"
        return
    if os.path.exists(fileName + ".h"):
        print "file: " + fileName + ".h exist"
        return
    cppFd = open(fileName + ".cpp", 'w')
    headerFd = open(fileName + ".h", 'w')

    classNameCutList = []
    index = 0
    lastIndex = 0
    for c in fileName:
        if c.isupper():
            if index > 0:
                print "upper " + "index = " + str(index) + " lastIndex = " + str(lastIndex)
                classNameCutList.append(fileName[lastIndex:index])
            lastIndex = index
        index += 1
    classNameCutList.append(fileName[lastIndex:])
    print classNameCutList
    headerStringList = []
    defineName = "__" + g_namespace.upper() + "_"
    for cn in classNameCutList:
        defineName += cn.upper() + "_"

    defineName += "H_INCLUDED__"
    print defineName
    
    headerStringList.append("#ifndef " + defineName + "\n")
    headerStringList.append("#define " + defineName + "\n")
    headerStringList.append("" + "\n")
    for iht in g_inheritList:
        l = iht.split(':', 1)
        headerStringList.append("#include \"" + l[0] + "\"\n")
    headerStringList.append("" + "\n")
    headerStringList.append("namespace " + g_namespace + "\n")
    headerStringList.append("{" + "\n")
    headerStringList.append("" + "\n")

    for cname in g_classList:
        inheritStr = ""
        for iht in g_inheritList:
            if inheritStr == "":
                inheritStr = " : public "
            else:
                inheritStr += " "
            l = iht.split(':', 1)
            inheritStr += l[1]
        headerStringList.append("class " + cname + inheritStr + "\n")
        headerStringList.append("{" + "\n")
        headerStringList.append("public:" + "\n")
        headerStringList.append("\t" + cname + "();" + "\n")
        headerStringList.append("\tvirtual ~" + cname + "();" + "\n")
        headerStringList.append("" + "\n")
        WriteHeaderFunction(headerStringList, cname)
        
        headerStringList.append("}; // class " + cname + "\n")
        headerStringList.append("" + "\n")
    headerStringList.append("} // namespace " + g_namespace + "\n")
    headerStringList.append("" + "\n")
    headerStringList.append("#endif // ifndef " + defineName + "\n")
    headerStringList.append("" + "\n")
    headerFd.writelines(headerStringList)
    headerFd.close()

    
    cppStringList = []
    cppStringList.append("#include \"" + g_namespace.capitalize() + ".h" + "\"" + "\n")
    cppStringList.append("" + "\n")
    cppStringList.append("#include " + "\"" + fileName + ".h" + "\"" + "\n")
    cppStringList.append("" + "\n")
    cppStringList.append("" + "\n")
    cppStringList.append("namespace " + g_namespace + "\n")
    cppStringList.append("{" + "\n")
    cppStringList.append("" + "\n")
    for cname in g_classList:
        cppStringList.append(cname + "::" + cname + "()\n")
        cppStringList.append("{" + "\n")
        cppStringList.append("}" + "\n")
        cppStringList.append("" + "\n")
        cppStringList.append(cname + "::~" + cname + "()\n")
        cppStringList.append("{" + "\n")
        cppStringList.append("}" + "\n")
        cppStringList.append("" + "\n")
        WriteCppFunction(cppStringList, cname)
    cppStringList.append("}" + " // namespace " + g_namespace + "\n")
    cppStringList.append("" + "\n")
    cppFd.writelines(cppStringList)
    cppFd.close()

def main(argv=None):
    if argv is None:
        argv = sys.argv
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "hf:c:u:i:", ["help", "file=", "class=", "func=", "inherit="])
        except getopt.error, msg:
            raise Usage(msg)
    except Usage, err:
        print >>sys.stderr, err.msg
        print >>sys.stderr, "for help use --help"
        return 2

    for name, value in opts:
        if name in ("-f", "--file"):
            AddFile(value)
        elif name in ("-c", "--class"):
            AddClass(value)
        elif name in ("-u", "--func"):
            AddFunc(value)
        elif name in ("-i", "--inherit"):
            AddInherit(value)
        elif name in ("-h", "--help"):
            PrintHelp()
    GenerateClass()

if __name__ == "__main__":
    sys.exit(main())

