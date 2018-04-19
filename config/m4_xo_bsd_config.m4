dnl XO_BSD_KERNLIBS()
dnl Tests for -lprop and headers.
AC_DEFUN([XO_BSD_KERNLIBS],[
    #-----------------------------------------
    # Check for assorted BSD "kernel" libraries
    #-----------------------------------------
    AC_CHECK_HEADER(uvm/uvm_extern.h,[
        AC_DEFINE(HAVE_UVM,[1],[Have UVM headers])])

    AC_CHECK_HEADER(sys/swap.h,[
        AC_CHECK_FUNC(swapctl,[
        AC_DEFINE(HAVE_SWAPCTL,[1],[Have swapctl])])])

    AC_CHECK_HEADER(prop/proplib.h,[
        AC_CHECK_LIB(prop, prop_object_release,[
            AC_DEFINE(HAVE_PROP,[1],[Have libprop])
            AX_APPEND_FLAG(-lprop, LIBS)])])
])


dnl XO_BSD_CONFIG()
dnl Test for BSD specific features.
AC_DEFUN([XO_BSD_CONFIG],[
    #---------------------------------
    # Tests specific to BSD variants.
    #---------------------------------
    AC_DEFINE(XOSVIEW_BSD,[1],[xosview bsd variant])

    XO_BSD_KERNLIBS()

    #-----------------------------------------
    # Add any platform specific things for the
    # bsd family of OSes
    #-----------------------------------------
    case $host_os in
    netbsd*)
        AC_DEFINE(XOSVIEW_NETBSD,[1],[xosview netbsd features])
        ;;

    dragonfly*)
        AC_DEFINE(XOSVIEW_DFBSD,[1],[xosview dragonfly bsd features])
        ;;

    freebsd*)
        AC_DEFINE(XOSVIEW_FREEBSD,[1],[xosview freebsd features])
	;;

    openbsd*)
        AC_CHECK_HEADERS([sys/dkstat.h sys/sched.h],[], [])
        AC_DEFINE(XOSVIEW_OPENBSD,[1],[xosview freebsd features])
	;;
    esac
])
