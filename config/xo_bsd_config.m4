#
# XOSView related tests specific to BSD variants
#
AC_DEFUN_ONCE([XO_BSD_CONFIG],[dnl

#  For FreeBSD, see if libdevstat exists.
case $host_os in
freebsd*)
  AC_CHECK_LIB(devstat, main,
  AC_DEFINE(HAVE_DEVSTAT,[1],[Have libdevstat])
  DEVSTATLIB=-ldevstat)
  ;;
esac

case $host_os in
##  Check for known versions of NetBSD...
dnl  Notice that we need to use [] to 'quote' the brackets --
dnl    according to GNU m4 and autoconf, [ and ] are the
dnl    quote characters.
dnl  This check is ugly.  Maybe it should be hidden away in aclocal?
dnl    bgrayson 9/96
netbsd1.[[0123456]]*|netbsdelf1.[[56]]*)
  ## Pull in NetBSD stuff:  set of host_os, INSTALL_ARGS, etc.
  AC_XOSV_NETBSD

  ## Check if the network interface supports 64-bit counters.  This
  ## support was committed around 11/19/1999, for 1.4P.
  if test $host_os_full '>' netbsd1.4O  -o $host_os_full '>' netbsdelf1.4O ; then
    AC_DEFINE(NETBSD_64BIT_IFACE_CTRS,[1],[NetBSD 64 bit network counters])
  fi

  ## We'll assume the existence of UVM now -- it's been around a long time.
  AC_DEFINE(UVM,[1],[UVM exists])
  host_dir=bsd

;;
##  Also check for versions that have not been tested.
netbsd*)
    # CURRENT CASE
    AC_XOSV_NETBSD
    host_dir=bsd
    echo host_dir is $host_dir
    host_os=netbsd ;;

freebsd[[0123]]*) AC_XOSV_FREEBSD
	dnl AC_MSG_WARN([
	dnl ])
	host_dir=bsd
	host_os=freebsd ;;
freebsd[[4]]*) AC_XOSV_FREEBSD
	AC_DEFINE(USE_KVM_GETSWAPINFO,[1],[Use libkvm getswapinfo])
	dnl AC_MSG_WARN([
	dnl ])
	host_dir=bsd
	host_os=freebsd ;;
openbsd2.[[0-5]]*) AC_XOSV_OPENBSD
	host_dir=bsd
	host_os=openbsd ;;
openbsd2.[[6-9]]*) AC_XOSV_OPENBSD
	cat >> confdefs.h <<\EOF
#define HAVE_SWAPCTL 1
EOF
	## Also check for the existence of UVM.  This is a nasty,
	## complicated check.  This could be avoided by having the *BSD
	## platform-specific Makefiles include <bsd.own.mk> on their own.

	cat > confmkfile <<\EOF

.include <bsd.own.mk>
all:
.if defined(UVM) && ${UVM:U} == YES
	@echo UVMyes
.endif
EOF

	uvmstring=`make -f confmkfile`
	rm -f confmkfile
	if test ! x$uvmstring = x ; then
		cat >> confdefs.h <<\EOF
#define UVM 1
EOF
	fi

	host_dir=bsd
	host_os=openbsd ;;
openbsd2.[[6-9]]*) AC_XOSV_OPENBSD
	cat >> confdefs.h <<\EOF
#define HAVE_SWAPCTL 1
EOF
	## Also check for the existence of UVM.  This is a nasty,
	## complicated check.  This could be avoided by having the *BSD
	## platform-specific Makefiles include <bsd.own.mk> on their own.

	cat > confmkfile <<\EOF

.include <bsd.own.mk>
all:
.if defined(UVM) && ${UVM:U} == YES
	@echo UVMyes
.endif
EOF

	uvmstring=`make -f confmkfile`
	rm -f confmkfile
	if test ! x$uvmstring = x ; then
		cat >> confdefs.h <<\EOF
#define UVM 1
EOF
	fi

	host_dir=bsd
	host_os=openbsd ;;
openbsd*) AC_XOSV_OPENBSD
	BTRYMETER=btrymeter.o
	cat >> confdefs.h <<\EOF
#define HAVE_SWAPCTL 1
#define UVM 1
#define HAVE_BATTERY_METER 1
#define APM_BATT_ABSENT APM_BATTERY_ABSENT
EOF

	host_dir=bsd
	host_os=openbsd ;;
bsdi*) AC_XOSV_BSDI
	host_dir=bsd
	host_os=bsdi ;;
*)       AC_MSG_ERROR([xosview has not been ported to $host_os :(.  Sorry.]) ;;
esac

dnl
#  Now check all the NetBSD ones for the new swapctl() function.
#  I don't know what the status of this is for OpenBSD folks.  Anyone?
#  Also look for battery-meter support.
#
case $host_os in
netbsd)
  AC_CHECK_FUNCS(swapctl)
  #  Only enable the battery meter if it appears we support it.
  if test -e /dev/apm ; then
    BTRYMETER=btrymeter.o
    AC_DEFINE(HAVE_BATTERY_METER,[1],[NetBSD battery meter])
  fi
  ;;
esac
])
