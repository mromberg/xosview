#
# Sets up AC_SUBST vars for gcc's auto depend
#
# Argument is a test that is "yes" or anything else false
# If "yes" then attempt to enable auto depend (if compiler supports it)
# If not "yes" still AC_SUBST the vars for no support
#

AC_DEFUN([XO_GCC_AUTO_DEPEND],[ dnl
CPP_DEP=""
AUTODEPEND=
AUTODEPRULE='dnl
##  Autodepend rule defaults to disabled.
#-include Makefile.GNU.autodep'
AUTODEPRULE_SUBDIR='dnl
##  Autodepend rule defaults to disabled.
#-include ../Makefile.GNU.autodep'

if test "$1" = "yes"; then
    CPP_DEP=""
    AUTODEPEND='-include $(DEPFILES)'
    # Check to see if we are using a gcc which
    # can make dependencies as a side effect of compilation.
    MY_CXX_SWITCH([[-Wp,-MD,depfile]])
    rm -f depfile
    if test "$my_cxx_switch" = "yes"; then
        m4_pushdef([my_dep_name],[[.$][@.d]])dnl
        CPP_DEP='-Wp,-MD,my_dep_name'
        m4_popdef([my_dep_name])dnl
    else
        AUTODEPRULE_SUBDIR='-include ../Makefile.GNU.autodep'
        AUTODEPRULE='-include Makefile.GNU.autodep'
    fi
fi

AC_SUBST(CPP_DEP)
AC_SUBST(AUTODEPEND)
AC_SUBST(AUTODEPRULE)
AC_SUBST(AUTODEPRULE_SUBDIR)
])
