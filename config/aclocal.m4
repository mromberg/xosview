#-------------------------------
# XO_CONCAT([SHVAR],[string1],[string2])
#
# Joins the two strings with a single space and stores them in the
# shell variable.  Won't add an space in the case string1 or
# string2 is empty "".
#-------------------------------
AC_DEFUN([XO_CONCAT],[dnl
if test -n "[$2]"; then
    if test -n "[$3]"; then
        $1="[$2] [$3]"
    else
        $1="[$2]"
    fi
else
    $1="[$3]"
fi
])


dnl
dnl This file containes a macro for each os xosview has been ported to.
dnl Each macro can add specific config options that apply to only that
dnl specific port.
dnl

dnl
dnl Checks for the existance of the C++ library
dnl files xosview expects to be there and useable
dnl The make headercheck target will search all .h and .cc
dnl files to help make/maintain this list
AC_DEFUN(CXX_LIBRARY_SUPPORT,[
AC_MSG_NOTICE([Examining C++ library support...])
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS(
[algorithm cassert cerrno cmath cstddef cstdlib cstring fstream iomanip iostream limits map sstream stdexcept string utility vector],
[],
[AC_MSG_WARN([Missing C++ library support.  Probably won't build.])])])


dnl Make an absolute symbol for the top of the configuration.
dnl
AC_DEFUN([CF_TOP_SRCDIR],
[TOP_SRCDIR=`cd $srcdir;pwd`
AC_SUBST(TOP_SRCDIR)
])dnl

AC_DEFUN(ICE_CXX_BOOL,
[
AC_REQUIRE([AC_PROG_CXX])
AC_MSG_CHECKING(whether ${CXX} supports bool types)
AC_CACHE_VAL(ice_cv_have_bool,
[
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[bool b = true;]])],[ice_cv_have_bool=yes],[ice_cv_have_bool=no])
AC_LANG_POP([])
])
AC_MSG_RESULT($ice_cv_have_bool)
if test "$ice_cv_have_bool" = yes; then
AC_DEFINE(HAVE_BOOL,[1],[Has type bool])
fi
])dnl

AC_DEFUN(ICE_CXX_LONG_LONG,
[
AC_REQUIRE([AC_PROG_CXX])
AC_MSG_CHECKING(whether ${CXX} supports long long types)
AC_CACHE_VAL(ice_cv_have_long_long,
[
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[long long x; x = (long long)0;]])],[ice_cv_have_long_long=yes],[ice_cv_have_long_long=no])
AC_LANG_POP([])
])
AC_MSG_RESULT($ice_cv_have_long_long)
if test "$ice_cv_have_long_long" = yes; then
AC_DEFINE(LONG_LONG,long long,[Have type long long])
else
AC_DEFINE(LONG_LONG,long,[long long is not so long])
fi
])dnl

dnl	For gcc-based (or primarily-gcc) OS's, set EXTRA_CXXFLAGS to -Wall -O4
AC_DEFUN(AC_GCC_EXTRA_CXXFLAGS, [
if test "$xosvdebug" = no; then
	XO_CONCAT(EXTRA_CXXFLAGS,[-W -Wall -O3],$EXTRA_CXXFLAGS)
else
      	XO_CONCAT(EXTRA_CXXFLAGS,[-W -Wall -g],$EXTRA_CXXFLAGS)
fi
AC_MSG_NOTICE([EXTRA_CXXFLAGS=$EXTRA_CXXFLAGS])
])

AC_DEFUN(SMP_LINUX,
[
AC_MSG_CHECKING(for SMP)
AC_EGREP_CPP(yes,
[#include <linux/autoconf.h>
#ifdef CONFIG_SMP
yes
#endif
], smp=yes, smp=no)
AC_MSG_RESULT($smp)
])dnl


AC_DEFUN(AC_SYS_LINUX_VERS,[[
changequote(<<, >>)
<<
LVERSION=`uname -r`
LVERSION=`expr $LVERSION : '\([0-9]*\.[0-9]*\)'`
>>
changequote([, ])
]])

AC_DEFUN(AC_XOSV_LINUX, [
EXTRALIBS="$EXTRALIBS $XPMLIB"

dnl
dnl Add a switch to add -DUSESYSCALLS for linux.
dnl
AC_ARG_ENABLE([linux-syscalls],
[  --enable-linux-syscalls use system calls when possible],

if test "$enableval" = "no"
then
        echo "disabled Linux system calls"
else
        AC_DEFINE(USESYSCALLS,[1],[Use Linux syscalls when possible])
        echo "enabled  Linux system calls"
fi
,
AC_DEFINE(USESYSCALLS)
echo "enabled  Linux system calls by default"
)

INSTALL_ARGS='-m 755'
]
	NetMeter_Default_Setting=True
)


dnl  ***  Below this line are the *BSD/BSDI and HPUX macros.  ***

dnl  ***  This one isn't actually used yet.  - bgrayson  ***
AC_DEFUN(AC_XOSV_BSD_COMMON, [
dnl  The BSD versions need to link with libkvm, and have the BSD install flags.
	EXTRALIBS="-lkvm $XPMLIB"
	INSTALL_ARGS='-s -g kmem -m 02555'
])

AC_DEFUN(AC_XOSV_NETBSD, [
dnl  Remember the full version in host_os_full
	host_os_full=$host_os
dnl  We need to strip the version numbers off the $host_os string (netbsd1.1)
dnl  Let's just be lazy -- set host_os to be netbsd.
	host_os=netbsd
dnl
dnl Netbsd needs to link with libkvm
dnl
        EXTRALIBS="-lkvm $XPMLIB"
        INSTALL_ARGS='-s -g kmem -m 02555'
	NetMeter_Default_Setting=True
	AC_DEFINE(XOSVIEW_NETBSD,[1],[NetBSD lkvm])
])

AC_DEFUN(AC_XOSV_FREEBSD, [
dnl
dnl FreeBSD also needs to link with libkvm
dnl
        EXTRALIBS="-lkvm $XPMLIB $DEVSTATLIB"
        INSTALL_ARGS='-s -g kmem -m 02555'
	NetMeter_Default_Setting=True
	AC_DEFINE(XOSVIEW_FREEBSD,[1],[FreeBSD lkvm])
])

AC_DEFUN(AC_XOSV_OPENBSD, [
dnl
dnl OpenBSD also needs to link with libkvm
dnl
        EXTRALIBS="-lkvm $XPMLIB"
        INSTALL_ARGS='-s -g kmem -m 02555'
	NetMeter_Default_Setting=True
	AC_DEFINE(XOSVIEW_OPENBSD,[1],[OpenBSD lkvm])
])

AC_DEFUN(AC_XOSV_BSDI, [
dnl
dnl BSDI (surprise, surprise) also needs to link with libkvm
dnl BSDI before 4.0 should probably have CXX=shlicc++ too so use
dnl gmake CXX=shlicc++ on bsdi [23].x
dnl
	EXTRALIBS="-lkvm $XPMLIB"
	INSTALL_ARGS='-s -g kmem -m 02555'
	NetMeter_Default_Setting=True
	AC_DEFINE(XOSVIEW_BSDI,[1],[BSDI lkvm])
])

AC_DEFUN(AC_XOSV_HPUX, [
dnl
dnl No special config options for HPUX.
dnl
])

AC_DEFUN(AC_XOSV_IRIX65, [
	dnl
	EXTRALIBS="-lrpcsvc"
    AC_DEFINE(_G_HAVE_BOOL,[1],[IRIX65 bool])
])

AC_DEFUN(AC_XOSV_GNU, [
EXTRALIBS=$XPMLIB
])

dnl MY_C_SWITCH(switch)
dnl -------------------
dnl try to compile and link a simple C program with the switch compile switch
dnl "${CC-cc} $CFLAGS $1 conftest.c -o conftest"
dnl sets my_cc_switch to switch if it worked
dnl my_cc_switch is not modified elsewhere

AC_DEFUN(MY_CXX_SWITCH,[
        AC_MSG_CHECKING(for [$1] as CXX compilation switch)
        cat > conftest.c <<__EOF
int main() { return 0;}
__EOF
        my_c='${CXX-cc} $CXXFLAGS $1 conftest.c -o conftest${ac_exeext}'
        (eval echo configure:__oline__: \"$my_c\") 1>&5
        (eval $my_c 1>/dev/null 2>conftest.log)
        cat conftest.log 1>&5
        if grep <conftest.log option  >/dev/null ||
           grep <conftest.log ERROR >/dev/null
        then
                my_cxx_switch="no"
                AC_MSG_RESULT(no)
        else
                my_cxx_switch="yes"
                AC_MSG_RESULT(yes)
        fi
        rm -rf conftest*
])
