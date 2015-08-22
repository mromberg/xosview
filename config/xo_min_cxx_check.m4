AC_DEFUN(XO_GCC_CXXFLAGS, [
#------------------------------------
# Set CXXFLAGS unless the user has
# already (env var etc)
#------------------------------------
if test "$GXX" = "yes"; then
    if test -z "$CXXFLAGS"; then
        CXXFLAGS="-Wall -Wextra"
    fi
    if test "$xosvdebug" = no; then
      	 XO_CONCAT(CXXFLAGS,$CXXFLAGS,[-O3])
    else
      	 XO_CONCAT(CXXFLAGS,$CXXFLAGS,[-g])
    fi
fi
])


AC_DEFUN_ONCE([XO_MIN_CXX_CHECK],[
#------------------------------------------------------
# Run some tests that should pass if there is
# any hope for xosview to build.  These tests
# check for the minumum amount of C++ compiler/library
# support.  They are not exhaustive.
#------------------------------------------------------
AC_MSG_NOTICE([Examining C++ library support...])
AC_TYPE_LONG_LONG_INT
AC_TYPE_UNSIGNED_LONG_LONG_INT
AC_TYPE_UINT64_T
dnl----------------------------------------------------
dnl make headercheck to create list of used C++ headers
dnl----------------------------------------------------
AC_CHECK_HEADERS([algorithm cassert cerrno cmath cmath cstddef cstdint dnl
    cstdlib cstring fstream iomanip iostream limits map set sstream    dnl
    stdexcept string utility vector],[],
    [AC_MSG_WARN([Missing C++ library support.])])

])
