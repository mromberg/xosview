AC_DEFUN(AC_XOSV_LINUX, [
# headers and funcs used by linux/serialmeter.cc
AC_CHECK_HEADERS([sys/io.h sys/perm.h asm/io.h])
AC_CHECK_FUNCS([ioperm])
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
])
