
TOP=$(PWD)

APP_PATH=$(TOP)/Src

ifeq "$(MAKECMDGOALS)" "rpm"
WORK_PATH=/usr/local/argenta
LIB_PATH=/usr/local/lib64
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
	cp $(ARGENTA_LIB) $(RPMDIR)$(LIB_PATH)/
	cp $(TOP)/Conf/argenta.conf $(RPMDIR)$(WORK_PATH)/conf/argenta.conf
	cp $(NGINX_PATH)/objs/nginx $(RPMDIR)$(WORK_PATH)/sbin/argenta

workspace:nginx
	make -C $(NGINX_PATH) install
	$(shell [ -d ${HOME}/lib ] || mkdir ${HOME}/lib )
	cd ${HOME}/lib; ln -sf $(TOP)/Build/CMake/libargenta.so libargenta.so
	ln -sf $(TOP)/Conf/argenta.conf $(TOP)/workspace/conf/argenta.conf

distclean:clean
	@rm -rf Build/CMake/CMakeFiles Build/CMake/cmake_install.cmake Build/CMake/CMakeCache.txt Build/CMake/Makefile

clean:
	-make -C $(NGINX_PATH) clean
	make -C Build/CMake clean
	@rm -rf $(WORK_PATH)
	@rm -rf $(RPMDIR)

install_rpm: nginx
	test -d '$(RPMDIR)' || mkdir -p '$(RPMDIR)'
	test -d '$(RPMDIR)$(WORK_PATH)/sbin' \
		|| mkdir -p '$(RPMDIR)$(WORK_PATH)/sbin'
	test ! -f '$(RPMDIR)$(WORK_PATH)/sbin/nginx' \
		|| mv '$(RPMDIR)$(WORK_PATH)/sbin/nginx' \
			'$(RPMDIR)$(WORK_PATH)/sbin/nginx.old'
	cp $(NGINX_PATH)/objs/nginx '$(RPMDIR)$(WORK_PATH)/sbin/nginx'
	test -d '$(RPMDIR)$(WORK_PATH)/conf' \
		|| mkdir -p '$(RPMDIR)$(WORK_PATH)/conf'
	cp $(NGINX_PATH)/conf/koi-win '$(RPMDIR)$(WORK_PATH)/conf'
	cp $(NGINX_PATH)/conf/koi-utf '$(RPMDIR)$(WORK_PATH)/conf'
	cp $(NGINX_PATH)/conf/win-utf '$(RPMDIR)$(WORK_PATH)/conf'
	test -f '$(RPMDIR)$(WORK_PATH)/conf/mime.types' \
		|| cp $(NGINX_PATH)/conf/mime.types '$(RPMDIR)$(WORK_PATH)/conf'
	cp $(NGINX_PATH)/conf/mime.types '$(RPMDIR)$(WORK_PATH)/conf/mime.types.default'
	test -f '$(RPMDIR)$(WORK_PATH)/conf/fastcgi_params' \
		|| cp $(NGINX_PATH)/conf/fastcgi_params '$(RPMDIR)$(WORK_PATH)/conf'
	cp $(NGINX_PATH)/conf/fastcgi_params \
		'$(RPMDIR)$(WORK_PATH)/conf/fastcgi_params.default'
	test -f '$(RPMDIR)$(WORK_PATH)/conf/fastcgi.conf' \
		|| cp $(NGINX_PATH)/conf/fastcgi.conf '$(RPMDIR)$(WORK_PATH)/conf'
	cp $(NGINX_PATH)/conf/fastcgi.conf '$(RPMDIR)$(WORK_PATH)/conf/fastcgi.conf.default'
	test -f '$(RPMDIR)$(WORK_PATH)/conf/uwsgi_params' \
		|| cp $(NGINX_PATH)/conf/uwsgi_params '$(RPMDIR)$(WORK_PATH)/conf'
	cp $(NGINX_PATH)/conf/uwsgi_params \
		'$(RPMDIR)$(WORK_PATH)/conf/uwsgi_params.default'
	test -f '$(RPMDIR)$(WORK_PATH)/conf/scgi_params' \
		|| cp $(NGINX_PATH)/conf/scgi_params '$(RPMDIR)$(WORK_PATH)/conf'
	cp $(NGINX_PATH)/conf/scgi_params \
		'$(RPMDIR)$(WORK_PATH)/conf/scgi_params.default'
	test -f '$(RPMDIR)$(WORK_PATH)/conf/nginx.conf' \
		|| cp $(NGINX_PATH)/conf/nginx.conf '$(RPMDIR)$(WORK_PATH)/conf/nginx.conf'
	cp $(NGINX_PATH)/conf/nginx.conf '$(RPMDIR)$(WORK_PATH)/conf/nginx.conf.default'
	cp $(ARGENTA_CONF) '$(RPMDIR)$(WORK_PATH)/conf/argenta.conf'
	test -d '$(RPMDIR)$(WORK_PATH)/logs' \
		|| mkdir -p '$(RPMDIR)$(WORK_PATH)/logs'
	test -d '$(RPMDIR)$(WORK_PATH)/logs' \
		|| mkdir -p '$(RPMDIR)$(WORK_PATH)/logs'
	test -d '$(RPMDIR)$(WORK_PATH)/html' \
		|| cp -R $(NGINX_PATH)/html '$(RPMDIR)$(WORK_PATH)'
	test -d '$(RPMDIR)$(WORK_PATH)/logs' \
		|| mkdir -p '$(RPMDIR)$(WORK_PATH)/logs'
	test -d '$(RPMDIR)$(LIB_PATH)' \
		|| mkdir -p '$(RPMDIR)$(LIB_PATH)'
	#test -d '$(SCRIPT_PATH)' \
		#|| mkdir -p '$(SCRIPT_PATH)' 

