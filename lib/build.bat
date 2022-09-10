@echo off
gcc -Wall -shared smbusb.c smbusb-fx2lp.c fxloader.c -DUSE_FX2LP_PROGRAMMER=1 -I../libusb -L../libusb -lusb-1.0 -olibsmbusb.dll
if %ERRORLEVEL% GTR 0 (
	echo Error building library
	exit 1
)