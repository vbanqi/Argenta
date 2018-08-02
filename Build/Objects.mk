
ALL_DIRS = $(shell find . -maxdepth 4 -type d)
MK_FILES = $(foreach eachdir, $(ALL_DIRS), $(wildcard $(eachdir)/$(MK_FILENAME)))
include $(MK_FILES)

SRC_DIRS = $(foreach filepath, $(MK_FILES), $(dir $(filepath)))
SRC_FILES = $(foreach module, $(MODULES), $(shell python ../Build/join_with_at.py $($(module).SRC)))
INC_FILES = $(foreach module, $(MODULES), $(shell python ../Build/join_with_at.py $($(module).INC)))
SRC_PATHS = $(shell python ../Build/join_source_file.py $(SRC_DIRS) $(SRC_FILES))
INC_PATHS = $(shell python ../Build/join_include_file.py $(SRC_DIRS) $(INC_FILES))

CPPFLAGS += $(INC_PATHS)

#OBJS = $(SRC_PATHS:.cpp=.o)

OBJ_TOP_DIR := $(BUILD_DIR)/obj
OBJ_DIRS := $(foreach srcdir, $(SRC_DIRS), $(OBJ_TOP_DIR)/$(srcdir))

CPPFLAGS += -I $(OBJ_TOP_DIR)
CPPFLAGS += -I /usr/local/ffmpeg/include -I../ThirdParty/opus-1.2.1/include
CPPFLAGS += -I../ThirdParty/fdk-aac/libAACdec/include -I../ThirdParty/fdk-aac/libAACenc/include
CPPFLAGS += -I../ThirdParty/fdk-aac/libSBRdec/include -I../ThirdParty/fdk-aac/libSBRenc/include 
CPPFLAGS += -I../ThirdParty/fdk-aac/libFDK/include -I../ThirdParty/fdk-aac/libSYS/include 

.PHONY: preobj clean release

ifneq "$(MAKECMDGOALS)" "clean"
OBJS = $(patsubst %.cpp, $(OBJ_TOP_DIR)/%.o, $(SRC_PATHS))
endif


#TARGET := libnspr.so

all: preobj $(TARGET)

release: all

preobj:
ifneq ($(OBJ_TOP_DIR), $(wildcard $(OBJ_TOP_DIR)))
	@echo $(OBJ_DIRS)
	@mkdir -p $(OBJ_DIRS)
endif

