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
        freetype2_inc="$x_includes/freetype2"
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
