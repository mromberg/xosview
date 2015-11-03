AC_DEFUN(XO_GCC_CXXFLAGS, [
#------------------------------------
# Set CXXFLAGS unless the user has
# already (env var etc)
#------------------------------------
if test "$GXX" = "yes"; then
    if test -z "$CXXFLAGS"; then
        # Turn on warnings
        CXXFLAGS="-Wall -Wextra"

        if test "$xosvdebug" = no; then
            # Optimize
      	    XO_CONCAT(CXXFLAGS,$CXXFLAGS,[-O3])
        else
            # Debug options
            XO_CONCAT(CXXFLAGS,$CXXFLAGS,[-g])

            # Debug libgc++ except for windows (where it blows up)
            if test "$host_os" != cygwin; then
                XO_CONCAT(CXXFLAGS,$CXXFLAGS,[-D_GLIBCXX_DEBUG])
            fi
        fi
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
AC_CHECK_HEADERS([algorithm cassert cctype cerrno cmath cstddef cstdlib cstring fstream iomanip iostream limits map set sstream stdexcept string utility vector],[],
    [AC_MSG_WARN([Missing C++ library support.])])

])
