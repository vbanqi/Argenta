#/bash/sh
pro=`pgrep $1`
echo $pro
pp=`echo $pro|sed 's/ /,/g'`
echo $pp
top -p $pp
