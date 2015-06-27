dnl
dnl Sets up AC_SUBST vars for gcc's auto depend
dnl
dnl Argument is a test that is "yes" or anything else false
dnl If "yes" then attempt to enable auto depend (if compiler supports it)
AC_DEFUN([MR_GCC_AUTO_DEPEND],[ dnl
if test "$1" = "yes"; then
CPP_DEP=""
dnl Check to see if we are using a gcc which
dnl can make dependencies as a side effect of compilation.
MY_CXX_SWITCH([[-Wp,-MD,depfile]])
rm -f depfile
if test "$my_cxx_switch" = "yes"
then
        m4_define([my_dep_name],[[.$][@.d]])
        CPP_DEP='-Wp,-MD,my_dep_name'
fi

if test "$enableval" = "no"
then
        AUTODEPEND=""
	AUTODEPRULE='\
##  Autodepend rule defaults to disabled.\
#-include Makefile.GNU.autodep'
	AUTODEPRULE_SUBDIR='\
##  Autodepend rule defaults to disabled.\
#-include ../Makefile.GNU.autodep'
else
        AUTODEPEND='-include $(DEPFILES)'
    if test "$my_cxx_switch" = "no"
    then
	AUTODEPRULE='-include Makefile.GNU.autodep'
	AUTODEPRULE_SUBDIR='-include ../Makefile.GNU.autodep'
    else
        AUTODEPRULE=""
        AUTODEPRULE_SUBDIR=""
    fi
fi
else
AUTODEPEND=
AUTODEPRULE='\
##  Autodepend rule defaults to disabled.\
#-include Makefile.GNU.autodep'
AUTODEPRULE_SUBDIR='\
##  Autodepend rule defaults to disabled.\
#-include ../Makefile.GNU.autodep'
fi

AC_SUBST(AUTODEPRULE)
AC_SUBST(AUTODEPRULE_SUBDIR)
AC_SUBST(AUTODEPEND)
AC_SUBST(CPP_DEP)
])
