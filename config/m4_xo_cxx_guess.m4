dnl XO_CXX_GUESS()
dnl If CXX is not set, create a list of possible canidates from
dnl executables found on $PATH.  Store list in xo_cxx_guess_list.
AC_DEFUN(XO_CXX_GUESS,[
    if test -z "$CXX"; then
        #-----------------------------------------
        # Create a list of possible C++ compilers
        # found on $PATH.
        #-----------------------------------------
        AC_MSG_CHECKING([for possible compilers])
        xo_comps=""
        for xo_vers in 10 9 8 7 6 5; do
            for xo_gxx in c++ g++ c++- g++-; do
                xo_comp=${xo_gxx}${xo_vers}
                which $xo_comp > /dev/null 2>&1
                if test "$?" != "1"; then
                    AX_APPEND_FLAG($xo_comp, xo_comps)
                fi
            done
        done
        # BSD and other systems usually have an old c++, g++ etc put them last.
        for xo_comp in c++ g++ clang++; do
            which $xo_comp > /dev/null 2>&1
            if test "$?" != "1"; then
                AX_APPEND_FLAG($xo_comp, xo_comps)
            fi
        done
        xo_cxx_guess_list="$xo_comps"
        AC_MSG_RESULT([$xo_cxx_guess_list])
    fi
])
