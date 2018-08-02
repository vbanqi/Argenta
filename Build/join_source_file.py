#!/usr/bin/python

import sys

itemNum = (len(sys.argv) - 1) / 2
pathArray = sys.argv[1:(itemNum + 1)]
fileArray = sys.argv[(itemNum + 1):]

for i in range(itemNum):
    if fileArray[i] == '@':
        continue
    else:
        for file in fileArray[i].split('@'):
            print pathArray[i] + file

