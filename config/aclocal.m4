dnl
dnl This file containes a macro for each os xosview has been ported to.
dnl Each macro can add specific config options that apply to only that
dnl specific port.
dnl
dnl $Id: aclocal.m4,v 1.4 1996/09/06 01:00:13 bgrayson Exp $
dnl

AC_DEFUN(AC_XOSV_LINUX, [
dnl
dnl Add a switch to add -DUSESYSCALLS for linux.
dnl
AC_ARG_ENABLE([linux-syscalls],
[  --enable-linux-syscalls use system calls when possible],

if test "$enableval" = "no"
then
        echo "disabled Linux system calls"
else
        AC_DEFINE(USESYSCALLS)
        echo "enabled  Linux system calls"
fi
,
AC_DEFINE(USESYSCALLS)
echo "enabled  Linux system calls by default"
)

dnl
dnl Add a switch which will build the memstat kernel module
dnl
AC_ARG_ENABLE([linux-memstat],
[  --enable-linux-memstat  build the linux memstat kernel module],

if test "$enableval" = "no"
then
        MEMSTAT=
        echo "disabled the Linux memstat module"
else
        MEMSTAT=MemStat
        echo "enabled  the Linux memstat module"
fi
,
MEMSTAT=
echo "disabled the Linux memstat module by default"
)
AC_SUBST(MEMSTAT)

INSTALL_ARGS='-s -m 4755'
])


AC_DEFUN(AC_XOSV_NETBSD, [
dnl  We need to strip the version numbers off the $host_os string (netbsd1.1)
dnl  Let's just be lazy -- set host_os to be netbsd.  
	host_os=netbsd
dnl
dnl Netbsd needs to link with libkvm
dnl
        EXTRALIBS=-lkvm
        INSTALL_ARGS='-s -g kmem -m 02555'
])


AC_DEFUN(AC_XOSV_HPUX, [
dnl
dnl No special config options for HPUX.
dnl
])
