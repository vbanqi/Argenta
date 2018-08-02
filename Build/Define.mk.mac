
BUILD_DIR := $(SRC_TOP)/../Build

CC := g++

CPPFLAGS := -Wall -Werror -std=c++11

ifdef RELEASE
CPPFLAGS +=  -O3 -DRELEASE
else
CPPFLAGS += -g -fno-common -I/usr//local/Cellar/openssl/1.0.2j/include
endif

LDFLAGS += -Wall -g #-O2 

MODULES :=

SOFLAGS := -dynamiclib -lavcodec -lavformat -lavfilter -lavdevice -lavresample -lavutil \
		   -lopencv_highgui -lopencv_core -lopencv_video -lopencv_imgproc -lssl -lcrypto

