AC_INIT([libsmbusb], [1.0.1], [contact@karosium.e4ward.com])

AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

LT_INIT
PKG_PROG_PKG_CONFIG([0.22])

SMB_LIB_VERSION_CURRENT=1
SMB_LIB_VERSION_REVISION=1
SMB_LIB_VERSION_AGE=0
SMB_LIB_VERSION="$SMB_LIB_VERSION_CURRENT:$SMB_LIB_VERSION_REVISION:$SMB_LIB_VERSION_AGE"
SMB_LIB_LDFLAGS="-version-info $SMB_LIB_VERSION"
AC_SUBST(SMB_LIB_VERSION_CURRENT)
AC_SUBST(SMB_LIB_VERSION_REVISION)
AC_SUBST(SMB_LIB_VERSION_AGE)
AC_SUBST(SMB_LIB_VERSION)
AC_SUBST(SMB_LIB_LDFLAGS)

test "$prefix" = NONE && prefix=/usr/local

# require libusb
PKG_CHECK_MODULES([libusb], [libusb-1.0 >= 1.0.5],
			[CFLAGS="$CFLAGS $libusb_CFLAGS";			
			LIBS="$LIBS $libusb_LIBS"])
AC_CHECK_LIB(usb-1.0, libusb_init)
#PKG_CHECK_MODULES([usb], [libusb-1.0 >= 1.0.9])

# require common stuff
AC_HEADER_STDC
AC_C_CONST
AC_TYPE_SIZE_T

AC_CHECK_FILE(fx2lib/Makefile, , [AC_MSG_ERROR(fx2lib is missing. did you run 'git submodule init' and 'git submodule update' after cloning?)])

# settings

SMB_CONF_DIRS=lib

AC_ARG_ENABLE([firmware],
  AS_HELP_STRING([--disable-firmware], [Don't build the firmware, use pre-built one]))

  AS_IF([test "x$enable_firmware" != "xno"], [
    AC_CHECK_PROG(xxd_found, xxd, yes)
    if test "$xxd_found" != yes ;then
            AC_MSG_ERROR("xxd is required for building the firmware")   
    fi
    AC_CHECK_PROG(sdcc_found, sdcc, yes)
    if test "$sdcc_found" != yes ;then
        AC_MSG_ERROR("SDCC is required for building the firmware")
    else
	has8051=`sdcc --version 2>&1 | grep "mcs51"`
	if test "$has8051" == "" ;then
		AC_MSG_ERROR("Installed SDCC doesn't support 8051 so can't built for EZUSB")
	fi
    fi
    
    SMB_CONF_DIRS="firmware $SMB_CONF_DIRS"
])

AC_ARG_ENABLE([fx2lp],
    AS_HELP_STRING([--disable-fx2lp], [Disable FX2LP probe driver]), [
      case "${enableval}" in
        yes) _fx2lp=true ;;
        no)  _fx2lp=false ;;
        *) AC_MSG_ERROR([bad value ${enableval} for --enable-fx2lp]) ;;
      esac
    ], [_fx2lp=true])
AM_CONDITIONAL([ENABLE_FX2LP], [test x$_fx2lp = xtrue])

AC_ARG_ENABLE([i2cdev],
    AS_HELP_STRING([--disable-i2cdev], [Disable i2c-dev probe driver]), [
      case "${enableval}" in
        yes) _i2cdev=true ;;
        no)  _i2cdev=false ;;
        *) AC_MSG_ERROR([bad value ${enableval} for --enable-i2cdev]) ;;
      esac
    ], [_i2cdev=true])
AM_CONDITIONAL([ENABLE_I2CDEV], [test x$_i2cdev = xtrue])

AC_ARG_ENABLE([tools],
    AS_HELP_STRING([--disable-tools], [Don't build the tools]))

AS_IF([test "x$enable_tools" != "xno"], [
  SMB_CONF_DIRS="$SMB_CONF_DIRS tools"
  AC_CONFIG_FILES([tools/Makefile])
])

AC_CONFIG_COMMANDS[patch -d fx2lib -p1 <fx2lib.patch -sN]

AC_SUBST(SMB_CONF_DIRS)

AC_CONFIG_FILES([lib/Makefile		 
		 lib/libsmbusb.pc
		 Makefile])

AC_MSG_NOTICE(-------------------------------)
AC_MSG_NOTICE([Build: $build])
AC_MSG_NOTICE([Host: $host])
AC_MSG_NOTICE([Compiler: $CC])
AC_MSG_NOTICE([Prefix: $prefix])

AS_IF([test "x$enable_firmware" == "xno"], [
  AC_CONFIG_COMMANDS([firmwarecopy],[cp ${srcdir}/firmware/firmware.h ${srcdir}/lib/])
  AC_MSG_NOTICE(* Using pre-built firmware)
])
AS_IF([test "x$enable_tools" == "xno"], [
  AC_MSG_NOTICE(* Tools will not be built)
])
AC_MSG_NOTICE(-------------------------------)

AC_OUTPUT


