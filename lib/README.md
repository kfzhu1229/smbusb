# libsmbusb

libsmbusb is a library for communicating with the SMBusb firmware. It automatically tries to upload the
firmware to the selected Cypress FX2LP device if it's not already running it.

### Functions:

#### Open/Close/Test

```c
int SMBOpenDevice(const char *device);
```

Open device by `device` definition. Definition is a URI-like:

* `fx2lp://vid=0x04b4,pid=0x8613`

* `fx2lp://bus=4,addr=16`

* `i2cdev:///dev/i2c-7`

* `i2c:///dev/i2c-7` - short alias to the previous one.

return value >=0 on success and contains the firmware version contained in the 3 lower bytes least signicant byte is most significant version number eg. 0x030001 = 1.0.3. I2CDEV driver just return zero here.

if <0 then error code, see libsmbusb.h

```c
void SMBCloseDevice();
```

```c
unsigned int SMBInterfaceId();
```

TBD: for I2CDEV

returns 0x4d5355 if firmware is responding properly. Zero for I2CDEV driver.

```c
void SMBSetDebugLogFunc(void *logFunc);
```

TBD: for I2CDEV

A pointer to a function with parameters (char * buf, int len) can be passed here to catch debug messages

#### Communication

##### Standard SMBus

```c
int SMBSendByte(unsigned int address, unsigned char command);
```

Sends the command byte immediately followed by STOP. Useful to invoke some parameterless commands.

```c
int SMBReadByte(unsigned int address, unsigned char command);
```

The standard SMBus Read Byte protocol. Reads an unsigned byte from "command"

```c
int SMBWriteByte(unsigned int address, unsigned char command, unsigned char data);
```

The standard SMBus Write Byte protocol. Writes a unsigned byte to "command":

```c
int SMBReadWord(unsigned int address, unsigned char command);
```

The standard SMBus Read Word protocol. Reads a 16bit word from "command"

```c
int SMBWriteWord(unsigned int address, unsigned char command, unsigned int data);
```

The standard SMBus Write Word protocol. Writes a 16bit word to "command"

```c
int SMBReadBlock(unsigned int address, unsigned char command, unsigned char *data);
```

The standard SMBus Read Block protocol. Reads a maximum of 255 bytes from "command"

```c
int SMBWriteBlock(unsigned int address, unsigned char command, unsigned char *data, unsigned char len);
```

The standard SMBus Write Block protocol. Writes a maximum of 255 bytes to "command"

TBD: SMBus limit block size by 32 bytes.

* Return values all for functions above will be >=0 on success. 
* Usually the number of bytes read for reads and 0 for writes.
* Values <0 are libusb error codes.
* Address parameters are always the READ address of the device.

```c
extern void SMBEnablePEC(unsigned char state);
```

0 disables, 1 enables SMBus Packet Error Checking. This is done in-firmware. For I2CDEV should be supported by the adapter, software emulation is not implemented.

When PEC is enabled reads will hard fail on PEC errors. Use SMBGetLastReadPECFail() to check whether hard-fails (retval < 0)  were due to a PEC mismatch.
    
Note that PEC is enabled by default and should be disabled manually if not needed. For I2CDEV driver state of PEC is unspecified. Best way to turn on/off it always.


```c
unsigned char SMBGetLastReadPECFail();
```

Returns 0xFF if PEC is enabled and the last read failed because of PEC, 0 otherwise

Calling this function also clears the PEC error flag so call it after every read when interested in PEC failure and it's location.

TBD: for I2CDEV

##### Arbitrary SMBus(/I2C)

```c
int SMBWrite(unsigned char start, unsigned char restart, unsigned char stop, 
             unsigned char *data, unsigned int len);
```

Write some bytes.

`start` and `restart` will generate the condition before sending "data" if ==1,

`stop` will send STOP after "data" is sent if ==1

```c
int SMBRead(unsigned int len, unsigned char* data, unsigned char lastRead);
```

Read some bytes.

Any number of bytes can be read. if lastRead is ==1 then STOP will be sent afterwards.
    
SMBWrite and SMBRead allow implementation of unstandard SMBus devices or I2C. Currently supported by the FX2LP only.

```c
struct SMBMsg {
    uint16_t addr;
    uint16_t flags;
#define SMB_M_RD       0x0001
#define SMB_M_NOSTART  0x4000
#define SMB_M_STOP     0x8000
    uint16_t len;
    uint8_t *buf;
};

int SMBTransfer(struct SMBMsg *msgs, size_t count);
```

Due to Linux i2c-dev does not support so much low level I2C access to emulate `SMBWrite` / `SMBRead` more common function is introduced. This solution too close to the `I2C_RDWR` IOCTL and `struct i2c_msg`. Does not supported by the FX2LP driver for now, but maybe software-emulated.


```c
unsigned int SMBGetArbPEC();
```

If PEC has been enabled with SMBEnablePEC(1) then PEC will be calculated on the fly when using SMBWrite and SMBRead commands. 

The firmware will read the extra PEC byte when SMBRead is called with lastRead==1 GetArbPec returns a 16bit word with the MSB being the PEC received from the device  and the LSB being the calculated PEC.
    
Note that not all devices will support PEC.
    
Note that PEC is enabled by default and should be disabled manually if not needed.

TBD: I2CDEV
