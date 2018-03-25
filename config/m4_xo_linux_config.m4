dnl --------------------------------------------
dnl Linux specific tests.
dnl --------------------------------------------
AC_DEFUN(AC_XOSV_LINUX, [
# headers and funcs used by linux/serialmeter.cc
AC_CHECK_HEADERS([sys/io.h])
AC_CHECK_FUNCS([ioperm])
#
# Add a switch to add -DUSESYSCALLS for linux.
#
AC_MSG_CHECKING([for use of linux system calls])
AC_ARG_ENABLE([linux-syscalls],
[  --enable-linux-syscalls use system calls when possible],

if test "$enableval" = "no"
then
        AC_MSG_RESULT([no])
else
        AC_DEFINE(USESYSCALLS,[1],[Use Linux syscalls when possible])
        AC_MSG_RESULT([yes])
fi
,
AC_DEFINE(USESYSCALLS)
AC_MSG_RESULT([yes])
)

INSTALL_ARGS='-m 755'
])
