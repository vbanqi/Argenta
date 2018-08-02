#!/bin/bash
 
test -d BUILD && rm -rf BUILD
test -d BUILDROOT && rm -rf BUILDROOT
test -d SPECS && rm -rf SPECS
test -d SOURCE && rm -rf SOURCE
test -d SRPMS && rm -rf SRPMS
test -d RPMS && rm -rf RPMS

if [ "$1" == "dev" ];then
./build_argenta-rpm.sh --prefix=/usr/local/argenta --name=-dev
elif [ "$1" == "prd" ];then 
./build_argenta-rpm.sh --prefix=/usr/local/argenta 
fi

exit 0
