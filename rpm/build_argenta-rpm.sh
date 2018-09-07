#!/bin/bash

POS_PREFIX=
POS_CONF=
POS_LIB=
POS_LOG=
POS_EXEC=
POS_NAME=

for option
do
    case "$option" in
        -*=*) value=`echo "$option" | sed -e 's/[-_a-zA-Z0-9]*=//'` ;;
           *) value="" ;;
    esac
    case "$option" in
        --prefix=)                       POS_PREFIX="!"             ;;
        --prefix=*)                      POS_PREFIX="$value"        ;;
        --lib=*)                         POS_LIB="$value"           ;;
        --name=*)                     	 POS_NAME="$value"          ;;
        *)
            echo "$0: error: invalid option \"$option\""
            exit 1
        ;;
    esac
done

#HOLA_GIT_PATH=git@10.2.250.21:rhea/argenta.git

export PATH=/opt/rh/devtoolset-4/root/usr/bin:/usr/kerberos/bin:/usr/lib/ccache:/usr/local/bin:/bin:/usr/bin:/home/build/bin:/sbin:/usr/sbin:/usr/local/argenta/sbin:/usr/local/argenta/lib

export PS1='[(argenta)\u@\h \W]$ '
export LANG=en_US.UTF-8

RPM_BUILD_PATH=`pwd`
RPM_DIR=${RPM_BUILD_PATH}
RPM_BUILD_ROOT=${RPM_DIR}

#BDIR=`date +%Y-%m-%d-%H-%M`;
#BDIR=2017-03-09-11-20

#echo $BDIR;
#mkdir -p $BDIR-argenta;
#cd $BDIR-argenta;
#pwd
#echo $HOLA_GIT_PATH
#umask 0002

BUILD_START=$(date +%s)

#echo "############### git start checkout code ################"
#git clone $HOLA_GIT_PATH 
#echo "############### git checkout success ###################"
if [ -n "$POS_PREFIX" ]; then
    sed -i "/WORK_PATH:=/c\WORK_PATH:=$POS_PREFIX" ../Makefile
fi
if [ -n "$POS_LIB" ]; then
    sed -i "/LIB_PATH:=/c\LIB_PATH:=$POS_LIB" ../Makefile
fi

cd ..; make distclean; make rpm; 
RESULT=$?
echo "The result of build is "${RESULT}
if [ "${RESULT}" != "0" ];
then
   echo "build failed, result is "${RESULT}
   exit -1;
fi 
echo "############### build finished ########################"

BUILD_END=$(date +%s)

REV=`git rev-list --all | wc -l`
if [ $? != "0" ];
then
	REV="0"
fi
echo $REV

echo "############### begin to make rpm #####################"
RPMSPEC=${RPM_DIR}/SPECS
RPMSOURCE=${RPM_DIR}/SOURCES
RPMDST=${RPM_DIR}/RPMS
mkdir -p ${RPMSPEC}
mkdir -p ${RPMSOURCE}
mkdir -p ${RPMDST}
cd ${RPMSOURCE}
mkdir -p fs_argenta
cd fs_argenta
TMP=`pwd`
/bin/cp -rf ${RPM_BUILD_PATH}/../rpmbuild/* .
#echo "${RPM_BUILD_PATH}/$BDIR-argenta/argenta/Source/rpm/*"
tar zcvf ../fs_argenta_bin.tar.gz . --exclude=".git*" 
cd ..
TMP=`pwd`
echo "fs_bin is here: $TMP"
/bin/rm -rf fs_argenta
cd ${RPMSPEC}
/bin/cp -f ${RPM_BUILD_PATH}/argenta_bin.spec .

REL_LINE=`grep -n "Release" argenta_bin.spec | cut -d: -f1 | sed  's/^[[:space:]]*//'`
echo "sed -i -e'${REL_LINE}c\Release : ${REV}' argenta_bin.spec"
echo "change rpm release version to git number"
sed -i -e''${REL_LINE}'c\Release : '${REV}'' argenta_bin.spec 

#echo "source ${RPM_BUILD_PATH}/${BDIR}-argenta/argenta/info.dat"
. ${RPM_BUILD_PATH}/../info.dat

APP_VERSION=$major.$minor.$revision
VER_LINE=`grep -n "Version" argenta_bin.spec | cut -d: -f1 | sed  's/^[[:space:]]*//'`
echo "sed -i -e'${VER_LINE}c\Version: ${APP_VERSION}' argenta_bin.spec"
sed -i -e''${VER_LINE}'c\Version: '${APP_VERSION}'' argenta_bin.spec

sed -i "/Packager/c\Packager: `git rev-list --all | head -1`" argenta_bin.spec

NAME_LINE=`grep -n "Name" argenta_bin.spec | cut -d: -f1 | sed  's/^[[:space:]]*//'`
echo "sed -i -e'${NAME_LINE}c\Name : ${POS_NAME}' argenta_bin.spec"
echo "change rpm name to assign name"
sed -i -e''${NAME_LINE}'c\Name : argenta-server'${POS_NAME}'' argenta_bin.spec 

rpmbuild --define "_topdir ${RPM_BUILD_PATH}" -vv -ba argenta_bin.spec
RESULT=$?
if [ "${RESULT}" != "0" ];
then
	echo "######################make rpm failed, result is "${RESULT}" ##############################"
	exit -1
else
	RPMNAME=`grep "Name" argenta_bin.spec | cut -d: -f2 | sed  's/^[[:space:]]*//'`
	RPMVER=`grep "Version" argenta_bin.spec | cut -d: -f2 | sed  's/^[[:space:]]*//'`
	RPMREL=`grep "Release" argenta_bin.spec | cut -d: -f2 | sed  's/^[[:space:]]*//'`
	HWPF=`uname -i | sed  's/^[[:space:]]*//'`
	RPMPACKAGE=${RPMNAME}-${RPMVER}-${RPMREL}.${HWPF}.rpm
	echo "dst rpm is "${RPMDST}/${HWPF}/${RPMPACKAGE}
	rm -f ${RPM_BUILD_PATH}/*.rpm
	mv ${RPMDST}/${HWPF}/${RPMPACKAGE} ${RPM_BUILD_PATH}
        
	#rm -rf ${RPM_BUILD_PATH}/${BDIR}-argenta/
    rm -rf ${RPM_BUILD_PATH}/RPMS
    rm -rf ${RPM_BUILD_PATH}/SPECS
    rm -rf ${RPM_BUILD_PATH}/SOURCES
    rm -rf ${RPM_BUILD_PATH}/BUILDROOT
fi
echo "######################## finish to make rpm ############################"

RPM_BUILD=$(date +%s)
BUILD_DIFF=$(($BUILD_END - $BUILD_START))
RPM_DIFF=$(($RPM_BUILD - $BUILD_END))

echo "###### build: $BUILD_DIFF s #########"
echo "###### rpm: $RPM_DIFF s #######"
echo "############### build success ###################################"


