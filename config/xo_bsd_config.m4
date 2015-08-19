#!/bin/sh

AC_DEFUN_ONCE([XO_BSD_KERNLIBS],[dnl
#-----------------------------------------
# Check for assorted BSD "kernel" libraries
#-----------------------------------------
AC_CHECK_HEADER(kvm.h)
AC_CHECK_LIB(kvm, kvm_open,
   [AC_DEFINE(HAVE_KVM,[1],[Have libkvm])
   XO_CONCAT(LIBS,$LIBS,[-lkvm])])

AC_CHECK_HEADER(prop/proplib.h)
AC_CHECK_LIB(prop, prop_object_release,
   [AC_DEFINE(HAVE_PROP,[1],[Have libprop])
    XO_CONCAT(LIBS,$LIBS,[-lprop])])

AC_CHECK_LIB(devstat, main,[dnl
             AC_DEFINE(HAVE_DEVSTAT,[1],[Have libdevstat])
             XO_CONCAT([LIBS],$LIBS,[-ldevstat])])

AC_CHECK_LIB(kinfo, kinfo_get_cpus,[dnl
             AC_DEFINE(HAVE_KINFO,[1],[Have libkinfo])
             XO_CONCAT([LIBS],$LIBS,[-lkinfo])])

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
