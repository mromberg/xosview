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


AC_DEFUN_ONCE(ICE_CXX_LONG_LONG,[
    AC_REQUIRE([AC_PROG_CXX])
    AC_MSG_CHECKING(whether ${CXX} supports long long types)
    AC_CACHE_VAL(ice_cv_have_long_long,[
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]],
            [[long long x; x = (long long)0;]])],
            [ice_cv_have_long_long=yes],[ice_cv_have_long_long=no])
    ])

    AC_MSG_RESULT($ice_cv_have_long_long)
    if test "$ice_cv_have_long_long" = yes; then
        AC_DEFINE(LONG_LONG,long long,[Have type long long])
    else
        AC_DEFINE(LONG_LONG,long,[long long is not so long])
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
ICE_CXX_LONG_LONG
AC_CHECK_HEADERS(
    [algorithm cassert cerrno cmath cstddef cstdlib cstring fstream iomanip iostream limits map sstream stdexcept string utility vector],
    [],[AC_MSG_WARN([Missing C++ library support.  Probably won't build.])])
])
