#-----------------------------------------------------------
#  This is just the spec file I use to update my loca yum
#  repository.  It may be usefule to look at for build
#  dependencies and installed files.
#-----------------------------------------------------------
Name:           xosview
Version:        1.8.4
Release:        1%{?dist}
Summary:        System resource monitor

License:        GPLv2, BSD
# RPM ingnores everything on the Source: line up to the last file name
# which must match the file to find in the SOURCES dir. RPM lint checks it.
# This is a perfect example of trying to overload one thing to do two jobs.
#Source: http://sourceforge.net/projects/gfed/files/1.2.0/gfe-1.2.0.tar.gz/download
Source: xosview-1.8.4.tar.gz
URL: https://sourceforge.net/projects/xosview/files/latest/download

BuildRequires: gawk libXext-devel gcc-c++ libXft-devel libXpm-devel

%description
xosview is a lightweight program that gathers information
from your operating system and displays it in graphical form.
It attempts to show you in a quick glance an overview of how
your system resources are being utilized.

%prep
%setup -q -n xosview-1.8.4

%build
configure --prefix=/usr
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

#-----------------------------
# local developer repo update
#-----------------------------
createrepo -c cache_dir --update ~/rpmbuild/RPMS
echo "RERUN createrepo -c cache_dir --update ~/rpmbuild/RPMS if --sign used"
#-----------------------------

%files
%defattr(-,root,root,-)
%{_bindir}/xosview
%{_mandir}/man1/xosview.1.gz
%{_datadir}/X11/app-defaults/XOsview
%{_docdir}/xosview/*

%changelog
* Sat Jul 11 2015 Mike Romberg <mike-romerg@comcast.net> 2.8-1
- Initial version of the package
