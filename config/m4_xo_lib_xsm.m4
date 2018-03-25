#
# Check for session management libs (-lSM -lICE)
#
AC_DEFUN([XO_CHECK_SM_LIBS],[dnl
    xo_cppflags="$CPPFLAGS"
    xo_ldflags="$LDFLAGS"
    CPPFLAGS="$X_CFLAGS"
    LDFLAGS="$X_LIBS"
    AC_CHECK_HEADER(X11/SM/SMlib.h,[dnl
        AC_CHECK_LIB(ICE, IceConnectionNumber,[dnl
            AC_CHECK_LIB(SM, SmsClientID,[dnl
                AC_DEFINE(HAVE_LIB_SM,[1],[Have libSM])],,-lICE)
                AX_APPEND_FLAG(-lSM, LIBS)
                AX_APPEND_FLAG(-lICE, LIBS)
        ])
    CPPFLAGS="$xo_cppflags"
    LDFLAGS="$xo_ldflags"
    ])
])
