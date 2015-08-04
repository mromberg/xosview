#!/bin/sh

#-------------------------------------------------------
# This script exists for the sole purpose of putting
# the version number from configure.in into xosview.spec
# at the time configure is created but before configure
# is run.
#-------------------------------------------------------

# Run m4 on configure.in with one macro
# that defines AC_INIT to spit out the version number
pkg_args="$(m4 << 'EOF'
divert(-1)dnl
changequote(`[', `]')
define([AC_INIT], [
define([PKGNAME_OUTPUT],[$1])
define([VERSION_OUTPUT],[$2])
define([PKGTARNAME_OUTPUT],[$4])])
include([configure.in])
divert(0)dnl
PKGNAME_OUTPUT VERSION_OUTPUT PKGTARNAME_OUTPUT
EOF
)"

n=0
for word in $pkg_args
do
    n=`expr $n + 1`
    eval arg$n="$word"
done
pkg_name="$arg1"
pkg_version="$arg2"
pkg_tarname="$arg3"

# Now pump xosview.spec.in through sed to inject the version number
sed_cmd="sed s/@PACKAGE_VERSION@/$pkg_version/;s/@PACKAGE_TARNAME@/$pkg_tarname/"

cat xosview.spec.in | $sed_cmd
