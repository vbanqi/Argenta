#!/bin/bash

BASE_DIR=.

CTAGS_PARAM='--sort=1 --c++-kinds=+p --fields=+iaS --extra=+q --language-force=C++'

CURRENT_DIR=`pwd`
TMP_TAGS_DIR=.

FILES_LIST=`find $CURRENT_DIR $@ -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.xml"`
cd $TMP_TAGS_DIR
rm -rf cscope.in.out  cscope.out  cscope.po.out tags
cscope -bkq $FILES_LIST
ctags $CTAGS_PARAM -f $TMP_TAGS_DIR/tags $FILES_LIST
cd $CURRENT_DIR

