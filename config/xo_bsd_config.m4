#!/bin/sh

AC_DEFUN_ONCE([XO_BSD_KERNLIBS],[dnl
#-----------------------------------------
# Check for assorted BSD "kernel" libraries
#-----------------------------------------
AC_CHECK_HEADER(uvm/uvm_extern.h,[dnl
    AC_DEFINE(HAVE_UVM,[1],[Have UVM headers])])

AC_CHECK_HEADER(sys/swap.h,[dnl
    AC_CHECK_FUNC(swapctl,[dnl
        AC_DEFINE(HAVE_SWAPCTL,[1],[Have swapctl])])])

AC_CHECK_HEADER(prop/proplib.h,[dnl
    AC_CHECK_LIB(prop, prop_object_release,[dnl
        AC_DEFINE(HAVE_PROP,[1],[Have libprop])
        XO_CONCAT(LIBS,$LIBS,[-lprop])])])

dnl AC_CHECK_LIB(kinfo, kinfo_get_cpus,[dnl
dnl    AC_DEFINE(HAVE_KINFO,[1],[Have libkinfo])
dnl    XO_CONCAT([LIBS],$LIBS,[-lkinfo])])
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
