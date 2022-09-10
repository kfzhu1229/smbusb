ACLOCAL_AMFLAGS = -I autostuff

lib_LTLIBRARIES = libsmbusb.la

libsmbusb_la_SOURCES = \
	libsmbusb.h \
	smbusb.c \
	smbusb_priv.h

SMB_LIB_CFLAGS =
SMB_LIB_LDFLAGS =

if ENABLE_FX2LP
libsmbusb_la_SOURCES += \
	smbusb-fx2lp.c \
	fxloader.c \
	firmware.h
SMB_LIB_CFLAGS += -DUSE_FX2LP_PROGRAMMER=1
endif

if ENABLE_I2CDEV
libsmbusb_la_SOURCES += \
	smbusb-i2cdev.c
SMB_LIB_CFLAGS += -DUSE_I2CDEV_PROGRAMMER=1
endif

libsmbusb_la_CFLAGS = $(SMB_LIB_CFLAGS)
libsmbusb_la_LDFLAGS = $(SMB_LIB_LDFLAGS)

libsmbusb_includedir=$(includedir)
libsmbusb_include_HEADERS = libsmbusb.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libsmbusb.pc

all: 
	cp ../firmware/firmware.h .
