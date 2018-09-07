打rpm包注意事项
1、首先需要在操作系统上安装rpm-build包
	yum -y install rpm-build
2、rmp包需要在centos系统上打才能在centos上运行

打包参数
1、--prefix参数如果不指定，默认为/usr/local/argenta
2、--conf参数不指定，默认为（--prefix）/conf
3、--lib参数不指定，默认为（--prefix）/lib
4、--exec参数不指定，默认为（--prefix）/sbin


打包具体过程
1、从git上下载相应源码
2、修改Source/Makefile_rpm，将参数加入到指定的位置
3、编译源码，即进入Source目录，执行make -f Makefile_rpm workspace
4、在rpm/rpmbuild(以下简称构建目录)目录下面创建SPECS、SOURCE、RPMS、SOURCE/fs_argenta目录
5、将第3步编译的文件，即Source/rmp目录下面的所有文件拷贝到rpm/rpmbuild/SOURCE/fs_argenta目录下
6、将fs_argenta目录打成gz压缩包
7、将构建目录下的argenta_bin.spec拷贝到构建目录下的SPECS中
8、在SPECS目下修改argenta_bin.spec文件中的Release为git提交次数，修改Version为根目录下的info.dat中指定的版本号，修改Packager为git最近提交的log标识
9、调用rpmbuild开始构建rpm包
10、删除git clone出的目录，RPMS，SPEC，SOURCE，BUILDROOT目录
11、至此rpm包建立完成，最终生成目录为rpm/rpmbuild目录下的tal-relayserver-x.x.x-x。x86_64.rpm
