# MY_C_SWITCH(switch)
# -------------------
# try to compile and link a simple C program with the switch compile switch
# "${CC-cc} $CFLAGS $1 conftest.c -o conftest"
# sets my_cc_switch to switch if it worked
# my_cc_switch is not modified elsewhere

AC_DEFUN(MY_CXX_SWITCH,[
        AC_MSG_CHECKING(for [$1] as CXX compilation switch)
        cat > conftest.c <<__EOF
int main() { return 0;}
__EOF
        my_c='${CXX-cc} $CXXFLAGS $1 conftest.c -o conftest${ac_exeext}'
        (eval echo configure:__oline__: \"$my_c\") 1>&5
        (eval $my_c 1>/dev/null 2>conftest.log)
        cat conftest.log 1>&5
        if grep <conftest.log option  >/dev/null ||
           grep <conftest.log ERROR >/dev/null
        then
                my_cxx_switch="no"
                AC_MSG_RESULT(no)
        else
                my_cxx_switch="yes"
                AC_MSG_RESULT(yes)
        fi
        rm -rf conftest*
])



#
# Sets up AC_SUBST vars for gcc's auto depend
#
# Argument is a test that is "yes" or anything else false
# If "yes" then attempt to enable auto depend (if compiler supports it)
# If not "yes" still AC_SUBST the vars for no support
#

AC_DEFUN([XO_AUTO_DEPEND],[ dnl
CPP_DEP=''
AUTODEPEND='#-include $(DEPFILES)'

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
    fi
fi

AX_APPEND_FLAG($CPP_DEP, CPPFLAGS)
AC_SUBST(AUTODEPEND)
])
