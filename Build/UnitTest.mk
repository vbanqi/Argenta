
ALL_DIRS = $(shell find . -maxdepth 4 -type d)
MODULES :=
UT_CPPFLAGS :=
UT_MK_FILES = $(foreach eachdir, $(ALL_DIRS), $(wildcard $(eachdir)/$(MK_FILENAME_UT)))
include $(UT_MK_FILES)

UT_SRC_DIRS = $(foreach filepath, $(UT_MK_FILES), $(dir $(filepath)))
UT_SRC_FILES = $(foreach module, $(MODULES), $(shell python ../Build/join_with_at.py $($(module).SRC)))
UT_INC_FILES = $(foreach module, $(MODULES), $(shell python ../Build/join_with_at.py $($(module).INC)))
UT_SRC_PATHS = $(shell python ../Build/join_source_file.py $(UT_SRC_DIRS) $(UT_SRC_FILES))
UT_INC_PATHS = $(shell python ../Build/join_include_file.py $(UT_SRC_DIRS) $(UT_INC_FILES))

OBJ_TOP_DIR := $(BUILD_DIR)/obj

OBJ_DIRS += $(foreach srcdir, $(UT_SRC_PATHS), $(OBJ_TOP_DIR)/$(dir $(srcdir)))


ifneq "$(MAKECMDGOALS)" "clean"
OBJS := $(patsubst %.cpp, $(OBJ_TOP_DIR)/%.o, $(UT_SRC_PATHS))
OBJS := $(patsubst %.cc, $(OBJ_TOP_DIR)/%.o, $(OBJS))
endif

NGX_OBJS = 

.PHONY: test preobj clean print

CPPFLAGS += $(UT_INC_PATHS)
CPPFLAGS += -I $(OBJ_TOP_DIR)

TEST = ut

print: preobj 
	@echo $(UT_SRC_DIRS)
	@echo $(UT_INC_PATHS)
	@echo $(UT_SRC_PATHS)

test: preobj $(TARGET) $(TEST)
	./ut

$(TEST): $(OBJS) $(NGX_OBJS)
	$(CC) $(LDFLAGS) -o $@ -Wl,--start-group $(OBJS) $(NGX_OBJS) $(LIBS) -Wl,--end-group -L$(SRC_TOP) -lpthread -lcrypto -lssl -lpcre -lcrypt -lrt

preobj:
ifneq ($(OBJ_TOP_DIR), $(wildcard $(OBJ_TOP_DIR)))
	@echo $(OBJ_DIRS)
	@mkdir -p $(OBJ_DIRS)
endif

