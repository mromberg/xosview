dnl XO_MSG_SECTION([message])
dnl Print a message for a section of tests.
AC_DEFUN(XO_MSG_SECTION, [
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([------- $1 -------------])
])


dnl XO_MSG_VAR([VAR])
dnl Displays VAR : $VAR using a field width of 8.
AC_DEFUN(XO_MSG_VAR, [
    AS_VAR_PUSHDEF([VAR], [m4_default($1,_AC_LANG_PREFIX[VAR])])
    AC_MSG_NOTICE(m4_format([%-8s : %s], VAR, [$VAR]))
    AS_VAR_POPDEF([VAR])
])
