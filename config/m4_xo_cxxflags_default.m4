dnl XO_CXXFLAGS_DEFAULT()
dnl Sets CXXFLAGS to default values if it has not been set in the env.
AC_DEFUN(XO_CXXFLAGS_DEFAULT, [
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
                AX_APPEND_FLAG(-O2, CXXFLAGS)
            else
                # Debug options
                AX_APPEND_FLAG(-g, CXXFLAGS)

                # Debug libgc++ except for windows (where it blows up)
                if test "$host_os" != cygwin; then
                    # Would like to add to DEFS but configure resets it.
                    AX_APPEND_FLAG(-D_GLIBCXX_DEBUG, CPPFLAGS)
                fi
            fi
        fi
    fi
    AC_MSG_CHECKING([for CXXFLAGS])
    AC_MSG_RESULT([$CXXFLAGS])
])
