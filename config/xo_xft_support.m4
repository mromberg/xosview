#
# Check for libXft
#
AC_DEFUN([XO_XFT_SUPPORT],[dnl
    AC_ARG_WITH([freetype2],[AS_HELP_STRING([--with-freetype2=DIR],
              [path to freetype2 headers])],[dnl
    freetype2_inc="$with_freetype2"],[dnl
    AC_PATH_PROG(FT_CONFIG,[freetype-config])
    if test -n "$FT_CONFIG"; then
        freetype2_inc=`$FT_CONFIG --prefix`
        freetype2_inc="$freetype2_inc/include/freetype2"
    else
        # In this case try and see if it is under X11
        # If X11 is on the standard search path of the compiler
        # and we are here we also don't have freetype-config
        # and the user did not help.  So, not even God can find
        # freetype2 'cause x_includes will be "".
        if test -n "$x_includes"; then
            AC_MSG_WARN([SWAG at freetype2 being in /usr/include/freetype2])
            freetype2_inc="/usr/include/freetype2"
        else
            freetype2_inc="$x_includes/freetype2"
        fi
    fi
    ])
    XFT_OBJS=""
    XOSV_FONT="7x13bold"
    CPPFLAGS="$CPPFLAGS -I$freetype2_inc"
        AC_CHECK_HEADERS([ft2build.h],[dnl
            AC_CHECK_HEADERS([X11/Xft/Xft.h],[dnl
            AC_CHECK_LIB(Xft, XftFontOpenName,[dnl
                AC_DEFINE(HAVE_XFT,[1],[Have libXft])
                XOSV_FONT="Oxygen Mono-8"
                XFT_OBJS="\$(XFT_OBJS)"
                XO_CONCAT(LIBS,$LIBS,[-lXft])
                ])
            ])
    ])
])
