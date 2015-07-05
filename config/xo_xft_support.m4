#
# Check for libXft
#
AC_DEFUN([XO_XFT_SUPPORT],[dnl
    AC_ARG_WITH([freetype2],[AS_HELP_STRING([--with-freetype2=DIR],
              [path to freetype2 headers [/usr/include/freetype2]])],[dnl
    freetype2_inc="$with_freetype2"],[freetype2_inc="/usr/include/freetype2"])
    XFT_OBJS=""
    XOSV_FONT="7x13bold"
    CPPFLAGS="$CPPFLAGS -I$freetype2_inc"
        AC_CHECK_HEADERS([ft2build.h],[dnl
            AC_CHECK_HEADERS([X11/Xft/Xft.h],[dnl
            AC_CHECK_LIB(Xft, XftFontOpenName,[dnl
                AC_DEFINE(HAVE_XFT,[1],[Have libXft])
                XOSV_FONT="Oxygen Mono-8"
                XFT_OBJS="\$(XFT_OBJS)"
                XO_CONCAT(EXTRA_CXXFLAGS,$EXTRA_CXXFLAGS,"-I$freetype2_inc")
                XO_CONCAT(EXTRALIBS,$EXTRALIBS, "-lXft")
                ])
            ])
    ])
])
