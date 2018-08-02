#!/usr/bin/python

import sys

#print 'Number of arguments:', len(sys.argv), 'arguments.'
#print 'Argument List:', str(sys.argv)

totalNumArgs = (len(sys.argv) - 1)
for index in range(totalNumArgs / 2 + 1):
    if index == 0:
        continue
    print str(sys.argv[index]) + str(sys.argv[index + totalNumArgs / 2]) + ' '

