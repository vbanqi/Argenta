
TOP=$(PWD)

APP_PATH=$(TOP)/Src

ifeq "$(MAKECMDGOALS)" "rpm"
WORK_PATH=/usr/local/argenta
RPMDIR:=$(TOP)/rpmbuild
ARGENTA_CONF:=$(TOP)/Conf/argenta.conf
else
WORK_PATH=$(TOP)/workspace
endif
ARGENTA_LIB:=$(TOP)/Build/CMake/libargenta.so
NGINX_PATH=$(TOP)/Nginx/nginx-1.10.3
NGINX_MODULE=nginx-module

NGX_TEST_CONFIG:=$(NGINX_PATH)/objs/ngx_auto_headers.h
.PHONY: all rpm nginx app workspace
	

all: workspace

./Build/CMake/Makefile:
	mkdir -p Build/CMake
	cd Build/CMake && cmake ../../Src

app:./Build/CMake/Makefile
	make -C Build/CMake

$(NGX_TEST_CONFIG):
	cd $(NGINX_PATH); ./configure --prefix=$(WORK_PATH) --with-debug --with-http_ssl_module --with-stream --with-stream_ssl_module --with-threads --add-module=../nginx-modules;

nginx:$(NGX_TEST_CONFIG) app 
	make -C $(NGINX_PATH)

rpm:nginx install_rpm
	chmod +x $(SCRIPT_FILES_PATH)/*
	cp $(ARGENTA_LIB) $(RPMDIR)$(LIB_PATH)/
	cp $(TOP)/Conf/argenta.conf $(RPMDIR)$(WORKSPACE)/conf/argenta.conf
	cp $(NGINX_PATH)/objs/nginx $(RPMDIR)$(WORKSPACE)/sbin/argenta

workspace:nginx
	make -C $(NGINX_PATH) install
	$(shell [ -d ${HOME}/lib ] || mkdir ${HOME}/lib )
	cd ${HOME}/lib; ln -sf $(TOP)/Build/CMake/libargenta.so libargenta.so
	ln -sf $(TOP)/Conf/argenta.conf $(TOP)/workspace/conf/argenta.conf

distclean:clean
	@rm -rf Build/CMake/CMakeFiles Build/CMake/cmake_install.cmake Build/CMake/CMakeCache.txt Build/CMake/Makefile

clean:
	make -C $(NGINX_PATH) clean
	make -C Build/CMake clean
	@rm -rf $(WORK_PATH)
	@rm -rf $(RPMDIR)

install_rpm: nginx
	test -d '$(RPMDIR)' || mkdir -p '$(RPMDIR)'
	test -d '$(RPMDIR)$(WORKSPACE)/sbin' \
		|| mkdir -p '$(RPMDIR)$(WORKSPACE)/sbin'
	test ! -f '$(RPMDIR)$(WORKSPACE)/sbin/nginx' \
		|| mv '$(RPMDIR)$(WORKSPACE)/sbin/nginx' \
			'$(RPMDIR)$(WORKSPACE)/sbin/nginx.old'
	cp $(NGINX_PATH)/objs/nginx '$(RPMDIR)$(WORKSPACE)/sbin/nginx'
	test -d '$(RPMDIR)$(WORKSPACE)/conf' \
		|| mkdir -p '$(RPMDIR)$(WORKSPACE)/conf'
	cp $(NGINX_PATH)/conf/koi-win '$(RPMDIR)$(WORKSPACE)/conf'
	cp $(NGINX_PATH)/conf/koi-utf '$(RPMDIR)$(WORKSPACE)/conf'
	cp $(NGINX_PATH)/conf/win-utf '$(RPMDIR)$(WORKSPACE)/conf'
	test -f '$(RPMDIR)$(WORKSPACE)/conf/mime.types' \
		|| cp $(NGINX_PATH)/conf/mime.types '$(RPMDIR)$(WORKSPACE)/conf'
	cp $(NGINX_PATH)/conf/mime.types '$(RPMDIR)$(WORKSPACE)/conf/mime.types.default'
	test -f '$(RPMDIR)$(WORKSPACE)/conf/fastcgi_params' \
		|| cp $(NGINX_PATH)/conf/fastcgi_params '$(RPMDIR)$(WORKSPACE)/conf'
	cp $(NGINX_PATH)/conf/fastcgi_params \
		'$(RPMDIR)$(WORKSPACE)/conf/fastcgi_params.default'
	test -f '$(RPMDIR)$(WORKSPACE)/conf/fastcgi.conf' \
		|| cp $(NGINX_PATH)/conf/fastcgi.conf '$(RPMDIR)$(WORKSPACE)/conf'
	cp $(NGINX_PATH)/conf/fastcgi.conf '$(RPMDIR)$(WORKSPACE)/conf/fastcgi.conf.default'
	test -f '$(RPMDIR)$(WORKSPACE)/conf/uwsgi_params' \
		|| cp $(NGINX_PATH)/conf/uwsgi_params '$(RPMDIR)$(WORKSPACE)/conf'
	cp $(NGINX_PATH)/conf/uwsgi_params \
		'$(RPMDIR)$(WORKSPACE)/conf/uwsgi_params.default'
	test -f '$(RPMDIR)$(WORKSPACE)/conf/scgi_params' \
		|| cp $(NGINX_PATH)/conf/scgi_params '$(RPMDIR)$(WORKSPACE)/conf'
	cp $(NGINX_PATH)/conf/scgi_params \
		'$(RPMDIR)$(WORKSPACE)/conf/scgi_params.default'
	test -f '$(RPMDIR)$(WORKSPACE)/conf/nginx.conf' \
		|| cp $(NGINX_PATH)/conf/nginx.conf '$(RPMDIR)$(WORKSPACE)/conf/nginx.conf'
	cp $(NGINX_PATH)/conf/nginx.conf '$(RPMDIR)$(WORKSPACE)/conf/nginx.conf.default'
	cp $(ARGENTA_CONF) '$(RPMDIR)$(WORKSPACE)/conf/argenta.conf'
	test -d '$(RPMDIR)$(WORKSPACE)/logs' \
		|| mkdir -p '$(RPMDIR)$(WORKSPACE)/logs'
	test -d '$(RPMDIR)$(WORKSPACE)/logs' \
		|| mkdir -p '$(RPMDIR)$(WORKSPACE)/logs'
	test -d '$(RPMDIR)$(WORKSPACE)/html' \
		|| cp -R $(NGINX_PATH)/html '$(RPMDIR)$(WORKSPACE)'
	test -d '$(RPMDIR)$(WORKSPACE)/logs' \
		|| mkdir -p '$(RPMDIR)$(WORKSPACE)/logs'
	test -d '$(RPMDIR)$(LIB_PATH)' \
		|| mkdir -p '$(RPMDIR)$(LIB_PATH)'
	#test -d '$(SCRIPT_PATH)' \
		#|| mkdir -p '$(SCRIPT_PATH)' 

