# Test if CXX compiler supports flag.
# $xo_cxx_flag_check will be either "yes" or "no".
AC_DEFUN([XO_CXX_FLAG_CHECK],[
    AC_MSG_CHECKING([if $CXX supports $1])
    AC_LANG_PUSH([C++])
    xo_saved_cxxflags="$CXXFLAGS"
    CXXFLAGS="-Werror $1"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
        [xo_cxx_flag_check="yes"],
        [xo_cxx_flag_check="no"])
    AC_MSG_RESULT([$xo_cxx_flag_check])
    CXXFLAGS="$xo_saved_cxxflags"
    AC_LANG_POP([C++])
])

#----------------------------------------------------------------------
# Sets up AC_SUBST vars for gcc's auto depend
#
# Argument is a test that is "yes" or anything else false
# If "yes" then attempt to enable auto depend (if compiler supports it)
# If not "yes" still AC_SUBST the vars for no support
#----------------------------------------------------------------------
AC_DEFUN([XO_AUTO_DEPEND],[
    CPP_DEP=''
    AUTODEPEND='#-include $(DEPFILES)'

    if test "$1" = "yes"; then
        CPP_DEP=""
        AUTODEPEND='-include $(DEPFILES)'
        # Check to see if we are using a gcc which
        # can make dependencies as a side effect of compilation.
        XO_CXX_FLAG_CHECK([-Wp,-MD,depfile])
        rm -f depfile
        if test "$xo_cxx_flag_check" = "yes"; then
            m4_pushdef([my_dep_name],[[.$][@.d]])dnl
            CPP_DEP='-Wp,-MD,my_dep_name'
            m4_popdef([my_dep_name])dnl
        fi
    fi

    AX_APPEND_FLAG($CPP_DEP, CPPFLAGS)
    AC_SUBST(AUTODEPEND)
])
