%define name MonetDB4-XQuery
%define version 0.36.3
%{!?buildno: %define buildno %(date +%Y%m%d)}
%define release %{buildno}%{?dist}%{?oid32:.oid32}%{!?oid32:.oid%{bits}}

# groups of related archs
%define all_x86 i386 i586 i686

%ifarch %{all_x86}
%define bits 32
%else
%define bits 64
%endif

Name: %{name}
Version: %{version}
Release: %{release}
Summary: MonetDB XQuery - Monet Database Management System
Vendor: MonetDB BV <monet@cwi.nl>

Group: Applications/Databases
License:   MPL - http://monetdb.cwi.nl/Legal/MonetDBLicense-1.1.html
URL: http://monetdb.cwi.nl/
Source: http://downloads.sourceforge.net/monetdb/MonetDB-XQuery-%{version}.tar.gz
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

Requires: MonetDB4-server >= 4.36
#                            ^^^^
# Maintained via vertoo. Please don't modify by hand!
# Contact MonetDB-developers@lists.sourceforge.net for details and/or assistance.

%{!?buildsystem: %define buildsystem 0}
%if !%{?buildsystem}
BuildRequires: MonetDB-devel >= 1.36
#                               ^^^^
# Maintained via vertoo. Please don't modify by hand!
# Contact MonetDB-developers@lists.sourceforge.net for details and/or assistance.
BuildRequires: MonetDB4-server-devel >= 4.36
#                                       ^^^^
# Maintained via vertoo. Please don't modify by hand!
# Contact MonetDB-developers@lists.sourceforge.net for details and/or assistance.
%endif
BuildRequires: libxml2-devel

%description
MonetDB is a database management system that is developed from a
main-memory perspective with use of a fully decomposed storage model,
automatic index management, extensibility of data types and search
accelerators, SQL- and XQuery- frontends.

This package contains the XQuery frontend.

%package devel
Summary: MonetDB XQuery development package
Group: Applications/Databases
Requires: %{name} = %{version}-%{release}
Requires: MonetDB-devel >= 1.36
#                          ^^^^
# Maintained via vertoo. Please don't modify by hand!
# Contact MonetDB-developers@lists.sourceforge.net for details and/or assistance.
Requires: MonetDB4-server-devel >= 4.36
#                                  ^^^^
# Maintained via vertoo. Please don't modify by hand!
# Contact MonetDB-developers@lists.sourceforge.net for details and/or assistance.
Requires: libxml2-devel

%description devel
MonetDB is a database management system that is developed from a
main-memory perspective with use of a fully decomposed storage model,
automatic index management, extensibility of data types and search
accelerators, SQL- and XQuery- frontends.

This package contains the files needed to develop with
MonetDB4-XQuery.

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n MonetDB-XQuery-%{version}

%build

%configure \
	--enable-strict=no \
	--enable-assert=no \
	--enable-debug=no \
	--enable-optimize=yes \
	--enable-bits=%{bits} \
	%{?oid32:--enable-oid32} \
	%{?comp_cc:CC="%{comp_cc}"} \
	--with-monet=%{_prefix}

make

%install
rm -rf $RPM_BUILD_ROOT

make install DESTDIR=$RPM_BUILD_ROOT

# cleanup stuff we don't want to install
rm -rf $RPM_BUILD_ROOT%{_libdir}/MonetDB5
find $RPM_BUILD_ROOT -name \*.la | xargs rm -f

# even remove DB2 specific stuff
rm -rf $RPM_BUILD_ROOT%{_bindir}/pfserialize.pl
rm -rf $RPM_BUILD_ROOT%{_bindir}/pfloadtables.sh
rm -rf $RPM_BUILD_ROOT%{_bindir}/pfcreatetables.sh

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/pf
%{_bindir}/pfopt
%{_bindir}/pfsql
%{_bindir}/pfshred
%{_libdir}/MonetDB4/lib/lib_*
%{_libdir}/MonetDB4/*.mil
%{_bindir}/Mbeddedxq
%{_libdir}/libembeddedxq.so.*
%dir %{_datadir}/MonetDB/xrpc
%dir %{_datadir}/MonetDB/xrpc/admin
%dir %{_datadir}/MonetDB/xrpc/export
%dir %{_datadir}/MonetDB/xrpc/demo
%{_datadir}/MonetDB/xrpc/admin/*
%{_datadir}/MonetDB/xrpc/export/*
%{_datadir}/MonetDB/xrpc/demo/*


%files devel
%defattr(-,root,root)
%{_libdir}/libembeddedxq.so
%{_bindir}/monetdb-xquery-config

%changelog
* Wed Feb 24 2010 Sjoerd Mullender <sjoerd@acm.org> - 0.36.1-20100224
- Rebuilt.

