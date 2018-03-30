dnl XO_PROG(program, var)
dnl Append program to var if program is found on  PATH
AC_DEFUN(XO_PROG,[
    AS_VAR_PUSHDEF([FLAGS], [m4_default($2,_AC_LANG_PREFIX[FLAGS])])
    xo_prog_var=`which $1 2>/dev/null`
    if test $? -eq 0 -a -n "$xo_prog_var"; then
        AX_APPEND_FLAG($1, FLAGS)
    fi
    AS_VAR_POPDEF([FLAGS])
])


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
                XO_PROG($xo_comp, xo_comps)
            done
        done
        # BSD and other systems usually have an old c++, g++ etc put them last.
        for xo_comp in c++ g++ clang++; do
            XO_PROG($xo_comp, xo_comps)
        done
        xo_cxx_guess_list="$xo_comps"
        AC_MSG_RESULT([$xo_cxx_guess_list])
    fi
])
