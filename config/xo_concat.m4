#------------------------------------------------------------------
# XO_CONCAT([SHVAR],[string1],[string2])
#
# Joins the two strings with a single space and stores them in the
# shell variable.  Won't add an space in the case string1 or
# string2 is empty "".
#------------------------------------------------------------------
AC_DEFUN([XO_CONCAT],[dnl
if test -n "[$2]"; then
    if test -n "[$3]"; then
        $1="[$2] [$3]"
    else
        $1="[$2]"
    fi
else
    $1="[$3]"
fi
])
