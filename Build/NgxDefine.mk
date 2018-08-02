
NGX_MAIN_DIR := ../Nginx/nginx-1.10.3
NGX_INCS := -I $(NGX_MAIN_DIR)/src/core \
		   -I $(NGX_MAIN_DIR)/src/event \
		   -I $(NGX_MAIN_DIR)/src/http \
		   -I $(NGX_MAIN_DIR)/src/http/modules \
		   -I $(NGX_MAIN_DIR)/src/event/modules \
		   -I $(NGX_MAIN_DIR)/src/os/unix \
		   -I $(NGX_MAIN_DIR)/src/stream \
		   -I $(NGX_MAIN_DIR)/objs \
		   -I ../Nginx/nginx-modules \
		   -I ../Nginx/nginx-rtmp-module

CPPFLAGS += $(NGX_INCS)

