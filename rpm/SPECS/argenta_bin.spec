Summary:   argenta package
Name : argenta-server-dev
Version: 1.0.0
Release : 0
License:   rokyou
Group:     System
Source:    fs_argenta_bin.tar.gz
Url:       https://github.com/vbanqi
Packager: 
Prefix:    %{_prefix}
Prefix:    %{_sysconfdir}
AutoReqProv: no
AutoReq:   no 
#BuildRequires:    gcc,make
Requires:       pcre,pcre-devel,openssl,chkconfig
%define    rootpath /
%define    usrpath %{rootpath}/usr
%define __prelink_undo_cmd     /bin/cat prelink library
%description
argenta

%prep
%setup -c

%install
rm -rf %{buildroot}
install -d $RPM_BUILD_ROOT%{rootpath}
pwd
cp -Rpd * $RPM_BUILD_ROOT%{rootpath}

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR
rm -rf %{_topdir}/SRPMS

%files
%defattr(-,root,root)
%{usrpath}
