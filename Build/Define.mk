
BUILD_DIR := $(SRC_TOP)/Build

CC := g++

CPPFLAGS := -Wall -Werror -std=c++11

ifdef RELEASE
CPPFLAGS +=  -g -O2 -fPIC -DRELEASE
else
CPPFLAGS +=  -g -fPIC
endif

LDFLAGS += -Wall -g #-O2 

MODULES :=

SOFLAGS := -fPIC -shared

