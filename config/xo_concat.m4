#------------------------------------------------------------------
# XO_CONCAT([SHVAR],[string1],[string2])
#
# Joins the two strings with a single space and stores them in the
# shell variable.  Won't add an space in the case string1 or
# string2 is empty "".
#------------------------------------------------------------------
AC_DEFUN([XO_CONCAT],[dnl
xo_cat_a=`echo $2`
xo_cat_b=`echo $3`
$1=`echo $xo_cat_a $xo_cat_b`
])
