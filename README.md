# smbusb
### Current status of the project:

* Project is forked from h4tr3d/smbusb
* M37512flasher has been fixed to work on CP2112, with a different block read algorithm implemented!
* BQ8030flasher right now is unmodified from that, has MAJOR ISSUES with random firmware erases with read operations, DO NOT CONNECT a BQ8030 in boot mode for now!
* R2J240flasher and everything else of this project are unmodified and mostly working as they should!

SMBusb is a USB SMBus interface based on the Cypress FX2LP.
This repository contains the firmware and library (libsmbusb) that make up the interface. 
Firmware is uploaded automatically by the library and remains loaded until power-cycle.

For more information see http://www.karosium.com/p/smbusb.html

Also, this tool-set supports any SMBus/I2C adapters on Linux via i2c-dev interface. Most common for the battery repair - CP2112.

### Prerequisites

* pkg-config
* libusb >= 1.0 for FX2LP driver

On *nix:

* build environment with autotools or CMake
  * CMake environment does not produce shared library for now
* sdcc and xxd for building the firmware [optional, pre-built firmware will be used]

On Windows:

* TDM-GCC (http://tdm-gcc.tdragon.net/) 
  
  Note: Windows build uses pre-built firmware
  
  Note: I2CDEV drivers is unsupported on Windows

### Build instructions

On *nix:

```shell
aclocal
libtoolize
autoconf
automake --add-missing
./configure (options: --disable-firmware, --disable-tools, --disable-fx2lp, --disable-i2cdev)
make
make install
```

Or:

```shell
mkdir build
cd build
cmake -DDISABLE_FIRMWARE=On \
      -DDISABLE_TOOLS=Off \
      -DENABLE_FX2LP_PROGRAMMER=On \
      -DENABLE_I2CDEV_PROGRAMMER=On
make
make install
```

On Windows:

```
build.bat
```

CMake build on Windows untested and unsupported for now. Any improvements are welcomed.
