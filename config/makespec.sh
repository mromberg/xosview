#!/bin/sh

#-------------------------------------------------------
# This script exists for the sole purpose of putting
# the version number from configure.in into xosview.spec
# at the time configure is created but before configure
# is run.
#-------------------------------------------------------

# Run m4 on configure.in with one macro
# that defines AC_INIT to spit out the version number
pkg_version="$(m4 << 'EOF'
divert(-1)dnl
changequote(`[', `]')
define([AC_INIT], [
define([VERSION_OUTPUT],[$2])])
include([configure.in])
divert(0)dnl
VERSION_OUTPUT
EOF
)"

# Now pump xosview.spec.in through sed to inject the version number
sed_cmd="sed s/@PACKAGE_VERSION@/$pkg_version/"

cat xosview.spec.in | $sed_cmd
