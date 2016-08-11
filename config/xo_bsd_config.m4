#!/bin/sh

AC_DEFUN_ONCE([XO_BSD_KERNLIBS],[dnl
#-----------------------------------------
# Check for assorted BSD "kernel" libraries
#-----------------------------------------
dnl AC_CHECK_HEADER(kvm.h)
dnl AC_CHECK_LIB(kvm, kvm_open,
dnl   [AC_DEFINE(HAVE_KVM,[1],[Have libkvm])
dnl   XO_CONCAT(LIBS,$LIBS,[-lkvm])])

AC_CHECK_HEADER(uvm/uvm_extern.h,[dnl
    AC_DEFINE(HAVE_UVM,[1],[Have UVM headers])])

AC_CHECK_HEADER(sys/swap.h,[dnl
    AC_CHECK_FUNC(swapctl,[dnl
        AC_DEFINE(HAVE_SWAPCTL,[1],[Have swapctl])])])

AC_CHECK_HEADER(prop/proplib.h,[dnl
    AC_CHECK_LIB(prop, prop_object_release,[dnl
        AC_DEFINE(HAVE_PROP,[1],[Have libprop])
        XO_CONCAT(LIBS,$LIBS,[-lprop])])])

AC_CHECK_LIB(devstat, main,[dnl
    AC_DEFINE(HAVE_DEVSTAT,[1],[Have libdevstat])
    XO_CONCAT([LIBS],$LIBS,[-ldevstat])])

AC_CHECK_LIB(kinfo, kinfo_get_cpus,[dnl
    AC_DEFINE(HAVE_KINFO,[1],[Have libkinfo])
    XO_CONCAT([LIBS],$LIBS,[-lkinfo])])
])


dnl Check to see if the SIOCGIFDATA ioctl seems to be supported.
dnl This new method of getting network stats appears in netbsd 7.0.0.
dnl Here we check for some headers, functions and symbols rather than
dnl using a version number.  Define XOSV_NETBSD_NET_IOCTL if supported.
AC_DEFUN([XO_NETBSD_NET_IOCTL],[dnl
    AC_CHECK_HEADER([net/if.h],[dnl
        AC_CHECK_FUNC(if_nameindex,xo_have_if_name_index="yes")])
    AC_CHECK_HEADER([sys/ioctl.h],[dnl
        AC_CHECK_DECL(SIOCGIFDATA,xo_have_SIOCGIFDATA="yes",[],[dnl
            AC_INCLUDES_DEFAULT([#include <sys/ioctl.h>])])])
    if test -n "$xo_have_if_name_index" -a -n "$xo_have_SIOCGIFDATA"; then
        AC_DEFINE(XOSV_NETBSD_NET_IOCTL,[1],[Use ioctl for net stats.])
    fi
])


#--------------------------------------------------
# XOSView related tests specific to BSD variants
#--------------------------------------------------
AC_DEFUN_ONCE([XO_BSD_CONFIG],[dnl

AC_DEFINE(XOSVIEW_BSD,[1],[xosview bsd variant])

XO_BSD_KERNLIBS

#-----------------------------------------
# Add any platform specific things for the
# bsd family of OSes
#-----------------------------------------
case $host_os in
    netbsd*)
        XO_NETBSD_NET_IOCTL
        ## We'll assume the existence of UVM now
        dnl AC_DEFINE(UVM,[1],[UVM exists])
        AC_DEFINE(XOSVIEW_NETBSD,[1],[xosview netbsd features])
        host_dir=bsd
        ;;

    dragonfly*)
        AC_DEFINE(XOSVIEW_DFBSD,[1],[xosview dragonfly bsd features])
        host_dir=bsd
        ;;

    freebsd*)
        AC_DEFINE(XOSVIEW_FREEBSD,[1],[xosview freebsd features])
	host_dir=bsd
	;;

    openbsd*)
        AC_CHECK_HEADERS([sys/dkstat.h sys/sched.h],[], [])
        AC_DEFINE(XOSVIEW_OPENBSD,[1],[xosview freebsd features])
	host_dir=bsd
	;;

    *)
        host_dir=bsd
        ;;
esac

])
