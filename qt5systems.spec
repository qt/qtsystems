Name:           qt5systems
Version:        5.0.0
Release:        1
License:        LGPL
Summary:        Qt5 Systems module
Url:            http://qt.nokia.com/
Source:         %name.tar.bz2
Patch1:         imei.patch
BuildRequires:  fdupes
BuildRequires:  gdb
BuildRequires:  gcc-c++
BuildRequires:  pam-devel
BuildRequires:  pcre-devel
BuildRequires:  tslib-devel
BuildRequires:  libqt5core
BuildRequires:  libqt5base-devel
BuildRequires:  libqt5declarative
BuildRequires:  libqt5declarative-devel
BuildRequires:  libqt5core-minimal
BuildRequires:  qt5jsondb-devel
BuildRequires:  mt-qt5-lib-devel
BuildRequires:  bluez-libs-devel
BuildRoot:      %{_tmppath}/%{name}-%{version}-build


%description
The Qt 5 Systems module

%package -n libqt5systeminfo
Summary:        Qt 5 SystemInfo library
Group:          System/Libraries
%description -n libqt5systeminfo
Qt 5 SystemInfo libraries

%package -n libqt5publishsubscribe
Summary:        Qt 5 Publish & Subscribe library
Group:          System/Libraries
%description -n libqt5publishsubscribe
Qt 5 Publish & Subscribe libraries

%package -n libqt5serviceframework
Summary:        Qt 5 Service Framework library
Group:          System/Libraries
%description -n libqt5serviceframework
Qt 5 Service Framework libraries

%package -n qt5servicefw
Summary:    Qt 5 Service Framework tool
Group:      Development/Tools
Requires:   libqt5serviceframework
%description -n qt5servicefw
Qt 5 Service Framework tool

%package -n libqt5systems-devel
Summary:        Qt 5 Systems development files
Group:          System/Libraries
Requires:       libqt5systeminfo
Requires:       libqt5publishsubscribe
Requires:       libqt5serviceframework
Requires:       qt5servicefw
%description -n libqt5systems-devel
Qt 5 Systems development files


%prep
%setup -q -n %{name}
%patch1 -p1

%build
# point to the location where the mkspecs can be found
export QTDIR=/usr/share/qt5 && qmake CONFIG+="no_wayland" "config_test_ofono=yes"

make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}

# Fix wrong path in pkgconfig files
find %{buildroot}%{_libdir}/pkgconfig -type f -name '*.pc' \
-exec perl -pi -e "s, -L%{_builddir}/%{name}/?\S+,,g" {} \;

# Fix wrong path in prl files
find %{buildroot}%{_libdir} -type f -name '*.prl' \
-exec sed -i -e "/^QMAKE_PRL_BUILD_DIR/d;s/\(QMAKE_PRL_LIBS =\).*/\1/" {} \;

rm -rf %{buildroot}%{_prefix}/tests
rm -f %{buildroot}%{_libdir}/libQt*.la

%clean


%post -n libqt5systeminfo -p /sbin/ldconfig

%postun -n libqt5systeminfo -p /sbin/ldconfig


%post -n libqt5publishsubscribe -p /sbin/ldconfig

%postun -n libqt5publishsubscribe -p /sbin/ldconfig


%post -n libqt5serviceframework -p /sbin/ldconfig

%postun -n libqt5serviceframework -p /sbin/ldconfig


%files -n libqt5systeminfo
%defattr(-,root,root)
%{_libdir}/libQtSystemInfo.so.*
%{_libdir}/qt5/imports/Qt/systeminfo

%files -n libqt5publishsubscribe
%defattr(-,root,root)
%{_libdir}/libQtPublishSubscribe.so.*
%{_libdir}/qt5/imports/Qt/publishsubscribe

%files -n libqt5serviceframework
%defattr(-,root,root)
%{_libdir}/libQtServiceFramework.so.*
%{_libdir}/qt5/imports/Qt/serviceframework

%files -n qt5servicefw
%defattr(-,root,root,-)
# >> files qt5servicefw
%{_bindir}/servicefw
# << files qt5servicefw

%files -n libqt5systems-devel
%defattr(-,root,root)
%{_includedir}/*
%{_libdir}/*.so
%{_datadir}/qt5/mkspecs/*
%{_libdir}/pkgconfig/*

%changelog
