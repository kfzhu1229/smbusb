/*
* Copyright (c) 2016 Viktor <github@karosium.e4ward.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

// https://en.cppreference.com/w/c/string/byte/memcpy
#define __STDC_WANT_LIB_EXT1__ 1

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/i2c-dev.h>
#ifndef I2C_M_RD
#  include <linux/i2c.h>
#endif
#include "libsmbusb.h"
#include "smbusb_priv.h"

#include <strings.h>
#include <stdbool.h>

struct i2c_ctx
{
	int fd;
	bool pec_enabled;
    bool sw_pec;
};

static struct i2c_ctx s_ctx = {-1, false, false};

static uint8_t pec_crc(uint8_t crc, uint8_t data)
{
    uint8_t i;

    data = crc ^ data;

    for ( i = 0; i < 8; i++ )
    {
        if (( data & 0x80 ) != 0 )
        {
            data <<= 1;
            data ^= 0x07;
        }
        else
        {
            data <<= 1;
        }
    }
    return (uint8_t)data;
}

static int xioctl(int fd, unsigned long int req, void *arg)
{
    int ret;
    while (true) {
        ret = ioctl(fd, req, arg);
        if (ret < 0 && (errno == EAGAIN || errno == EINTR)) {
            continue;
        }
        break;
    }
    return ret;
}

void (*extLogFunc)(unsigned char* buf, unsigned int len) = NULL;

static int I2CDEV_SMBOpenDeviceI2c(const char *dev_name)
{
	s_ctx.fd = open(dev_name, O_RDWR);
	if (s_ctx.fd < 0)
		return -1;

    unsigned long funcs;
    int sts = xioctl(s_ctx.fd, I2C_FUNCS, &funcs);
    if (sts)
        return sts;

    fprintf(stderr, "I2C controller functionality:\n");

#define FUNC_STR(x, name, prefix) \
    if ((funcs & (x)) == (x)) \
            fprintf(stderr, "%s%s\n", prefix, name)
#define FUNC_PREFIX(x, prefix) FUNC_STR(x, #x, prefix)
#define FUNC(x)                FUNC_STR(x, #x, "  ")

    FUNC(I2C_FUNC_I2C);
    FUNC(I2C_FUNC_10BIT_ADDR);
    FUNC(I2C_FUNC_PROTOCOL_MANGLING);
    FUNC(I2C_FUNC_SMBUS_PEC);
    FUNC(I2C_FUNC_NOSTART);
    FUNC(I2C_FUNC_SLAVE);
    FUNC(I2C_FUNC_SMBUS_BLOCK_PROC_CALL);
    FUNC(I2C_FUNC_SMBUS_QUICK);
    FUNC(I2C_FUNC_SMBUS_READ_BYTE);
    FUNC(I2C_FUNC_SMBUS_WRITE_BYTE);
    FUNC(I2C_FUNC_SMBUS_READ_BYTE_DATA);
    FUNC(I2C_FUNC_SMBUS_WRITE_BYTE_DATA);
    FUNC(I2C_FUNC_SMBUS_READ_WORD_DATA);
    FUNC(I2C_FUNC_SMBUS_WRITE_WORD_DATA);
    FUNC(I2C_FUNC_SMBUS_PROC_CALL);
    FUNC(I2C_FUNC_SMBUS_READ_BLOCK_DATA);
    FUNC(I2C_FUNC_SMBUS_WRITE_BLOCK_DATA);
    FUNC(I2C_FUNC_SMBUS_READ_I2C_BLOCK);
    FUNC(I2C_FUNC_SMBUS_WRITE_I2C_BLOCK);
    FUNC(I2C_FUNC_SMBUS_HOST_NOTIFY);

    FUNC_PREFIX(I2C_FUNC_SMBUS_BYTE, "    ");
    FUNC_PREFIX(I2C_FUNC_SMBUS_BYTE_DATA, "    ");
    FUNC_PREFIX(I2C_FUNC_SMBUS_WORD_DATA, "    ");
    FUNC_PREFIX(I2C_FUNC_SMBUS_BLOCK_DATA	, "    ");
    FUNC_PREFIX(I2C_FUNC_SMBUS_I2C_BLOCK	, "    ");
    FUNC_PREFIX(I2C_FUNC_SMBUS_EMUL, "    ");

#undef FUNC
#undef FUNC_PREFIX

    s_ctx.sw_pec = !(funcs & I2C_FUNC_SMBUS_PEC);

	return 0;
}

static void I2CDEV_SMBCloseDevice(void)
{
	close(s_ctx.fd);
	s_ctx.fd = -1;
	s_ctx.pec_enabled = true;
}

static inline int smbus_access(int fd, char read_write, uint8_t command, int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;
	int ret = xioctl(fd, I2C_SMBUS, &args);

	if (ret) {
		fprintf(stderr, "SMBus error: %d\n", errno);
	}

	return ret;
}

static inline int smbus_pec(int fd, bool enable)
{
    if (s_ctx.sw_pec) {
        s_ctx.pec_enabled = enable;
        //return 0;
    }

	long select = enable;
	if (xioctl(fd, I2C_PEC, (void*)select)) {
		fprintf(stderr, "smbus_pec(): error %d\n", errno);
		return -1;
	}
	return 0;
}

static inline int smbus_address(int fd, uint16_t address)
{
    return xioctl(fd, I2C_SLAVE, (void*)(unsigned long)(address >> 1));
}

//
// S Addr Rd/Wr [A] P
//
static int smbus_write_quick(int fd, uint8_t value)
{
	return smbus_access(fd, value, 0, I2C_SMBUS_QUICK, NULL);
}

//
// S Addr Rd [A] [Data] NA P
//
static int smbus_read_byte(int fd)
{
	union i2c_smbus_data data;
	int err;

	err = smbus_access(fd, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data);
	if (err < 0)
		return err;

	return 0x0FF & data.byte;
}

//
// S Addr Wr [A] Data [A] P
//
static int smbus_write_byte(int fd, uint8_t value)
{
	return smbus_access(fd, I2C_SMBUS_WRITE, value, I2C_SMBUS_BYTE, NULL);
}

//
// S Addr Wr [A] Comm [A] Sr Addr Rd [A] [Data] NA P
//
static int smbus_read_byte_data(int fd, uint8_t command)
{
	union i2c_smbus_data data;
	int err;

	err = smbus_access(fd, I2C_SMBUS_READ, command, I2C_SMBUS_BYTE_DATA, &data);
	if (err < 0)
		return err;

	return 0x0FF & data.byte;
}

//
// S Addr Wr [A] Comm [A] Data [A] P
//
static int smbus_write_byte_data(int fd, uint8_t command, uint8_t value)
{
	union i2c_smbus_data data;
	data.byte = value;
	return smbus_access(fd, I2C_SMBUS_WRITE, command, I2C_SMBUS_BYTE_DATA, &data);
}

//
// S Addr Wr [A] Comm [A] Sr Addr Rd [A] [DataLow] A [DataHigh] NA P
//
static int smbus_read_word_data(int fd, uint8_t command)
{
	union i2c_smbus_data data;
	int err;

 	err = smbus_access(fd, I2C_SMBUS_READ, command, I2C_SMBUS_WORD_DATA, &data);
	if (err < 0)
		return err;

	return 0x0FFFF & data.word;
}

//
// S Addr Wr [A] Comm [A] DataLow [A] DataHigh [A] P
//
static int smbus_write_word_data(int file, uint8_t command, uint16_t value)
{
	union i2c_smbus_data data;
	data.word = value;
	return smbus_access(file, I2C_SMBUS_WRITE, command, I2C_SMBUS_WORD_DATA, &data);
}


//
// S Addr Wr [A] Comm [A]
//                        Sr Addr Rd [A] [Count] A [Data] A [Data] A ... A [Data] NA P
//
static int smbus_read_block_data(int fd, uint8_t command, uint8_t *values)
{
	union i2c_smbus_data data;
	int i, err;

	err = smbus_access(fd, I2C_SMBUS_READ, command, I2C_SMBUS_BLOCK_DATA, &data);
	if (err < 0)
		return err;

#if 1
	for (i = 1; i <= data.block[0]; i++)
		values[i-1] = data.block[i];
#else
	memcpy(values, &data.block[1], data.block[0]);
#endif
	return data.block[0];
}

//
// S Addr Wr [A] Comm [A] Count [A] Data [A] Data [A] ... [A] Data [A] P
//
static int smbus_write_block_data(int fd, uint8_t command, uint8_t length, const uint8_t *values)
{
	union i2c_smbus_data data;
	int i;
	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];
	data.block[0] = length;
	return smbus_access(fd, I2C_SMBUS_WRITE, command, I2C_SMBUS_BLOCK_DATA, &data);
}

//
// S Addr Wr [A] Comm [A]
//                        Sr Addr Rd [A] [Data] A [Data] A ... A [Data] NA P
//
static int smbus_read_i2c_block_data(int fd, uint8_t command, uint8_t length, uint8_t *values)
{
	union i2c_smbus_data data;
	int i, err;

	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	data.block[0] = length;

	err = smbus_access(fd, I2C_SMBUS_READ, command, length == 32 ? I2C_SMBUS_I2C_BLOCK_BROKEN : I2C_SMBUS_I2C_BLOCK_DATA, &data);
	if (err < 0)
		return err;

	for (i = 1; i <= data.block[0]; i++)
		values[i-1] = data.block[i];
	return data.block[0];
}

//
// S Addr Wr [A] Comm [A] Data [A] Data [A] ... [A] Data [A] P
//
static int smbus_write_i2c_block_data(int fd, uint8_t command, uint8_t length, const uint8_t *values)
{
	union i2c_smbus_data data;
	int i;
	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];
	data.block[0] = length;
	return smbus_access(fd, I2C_SMBUS_WRITE, command, I2C_SMBUS_I2C_BLOCK_BROKEN, &data);
}

//
// Relax requirements for the MAX SMBus block size
//
// S Addr Wr [A] Comm [A]
//                        Sr Addr Rd [A] [Count] A [Data] A [Data] A ... A [Data] NA P
//
static int smbus_read_block_data_relax(int fd, uint8_t address, uint8_t command, uint8_t *values)
{

    struct i2c_msg msg[2];
    struct i2c_rdwr_ioctl_data req = {
        .msgs = msg,
        .nmsgs = sizeof(msg) / sizeof(*msg),
    };

    uint8_t buffer[256+2] = {}; // Data + Size + PEC

    // Command write transaction
    msg[0].addr = address>>1;
    msg[0].flags = 0;
    msg[0].buf = &command;
    msg[0].len = 1;

    // Data transaction
    msg[1].addr = address>>1;
    msg[1].flags = I2C_M_RD;
    msg[1].buf = buffer;
    msg[1].len = 1; // read only size field

    int sts = xioctl(fd, I2C_RDWR, &req);
    if (sts < 0) {
        fprintf(stderr, "smbus_read_block_data_relax: error %d\n", errno);
        return -1;
    }

    //msg[1].len = buffer[0] + (s_ctx.pec_enabled ? 1 : 0);
    msg[1].len = buffer[0];
    sts = xioctl(fd, I2C_RDWR, &req);
    if (sts < 0) {
        fprintf(stderr, "smbus_read_block_data_relax: error %d\n", errno);
        return -1;
    }

#if 0
    if (s_ctx.pec_enabled) {
        uint8_t pec = 0;
        pec = pec_crc(pec, address);
        pec = pec_crc(pec, command);
        pec = pec_crc(pec, address+1);

        for (int i = 0; i <= buffer[0]; ++i) {
            pec = pec_crc(pec, buffer[i]);
            fprintf(stderr, "%02x ", buffer[i]);
        }
        fprintf(stderr, "%02x\n", buffer[buffer[0]+1]);

        if (pec != buffer[buffer[0]+1]) {
            errno = EBADMSG;
            return -1;
        }
    }
#endif

    // Copyout data
    memcpy(values, &buffer[1], buffer[0]);

    return buffer[0];
}

//
// S Addr Wr [A] Comm [A] Count [A] Data [A] Data [A] ... [A] Data [A] P
//
static int smbus_write_block_data_relax(int fd, uint8_t address, uint8_t command, uint8_t length, const uint8_t *values)
{
    struct i2c_msg msg[1];
    struct i2c_rdwr_ioctl_data req = {
        .msgs = msg,
        .nmsgs = sizeof(msg) / sizeof(*msg),
    };

    uint8_t buffer[256+3] = {}; // Data + Command + Size + PEC

    buffer[0] = command;
    buffer[1] = length;

    memcpy(&buffer[2], values, length);

    // Command write transaction
    msg[0].addr = address>>1;
    msg[0].flags = 0;
    msg[0].buf = buffer;
    msg[0].len = length + 2; // TBD: PEC checks

    int sts = xioctl(fd, I2C_RDWR, &req);
    if (sts < 0) {
        fprintf(stderr, "smbus_read_block_data_relax: error %d\n", errno);
        return -1;
    }

    return 0;
}

static int I2CDEV_SMBReadByte(unsigned int address, unsigned char command)
{
	int ret = smbus_address(s_ctx.fd, address);
	if (ret < 0)
        return ret;

#if 0
    if (s_ctx.pec_enabled) {
        uint8_t buf[2] = {};
        uint8_t pec = 0;

        if (smbus_read_i2c_block_data(s_ctx.fd, command, 2, buf))
            return -1;

        pec = pec_crc(pec, address);
        pec = pec_crc(pec, command);
        pec = pec_crc(pec, address+1);
        pec = pec_crc(pec, buf[0]);
        if (pec != buf[1]) {
            fprintf(stderr, "PEC CRC failed\n");
            return -1;
        }
        return buf[0];
    }
#endif

	return smbus_read_byte_data(s_ctx.fd, command);

}

static int I2CDEV_SMBSendByte(unsigned int address, unsigned char command)
{
	int ret = smbus_address(s_ctx.fd, address);
	if (ret < 0)
		return ret;

    if (s_ctx.pec_enabled) {
        //return
    }

	return smbus_write_byte(s_ctx.fd, command);
}


static int I2CDEV_SMBWriteByte(unsigned int address, unsigned char command, unsigned char data)
{
	int ret = smbus_address(s_ctx.fd, address);
	if (ret < 0)
		return ret;
	return smbus_write_byte_data(s_ctx.fd, command, data);
}


static int I2CDEV_SMBReadWord(unsigned int address, unsigned char command)
{
	int ret = smbus_address(s_ctx.fd, address);
	if (ret < 0)
		return ret;
	return smbus_read_word_data(s_ctx.fd, command);
}

static int I2CDEV_SMBWriteWord(unsigned int address, unsigned char command, unsigned int data)
{
	int ret = smbus_address(s_ctx.fd, address);
	if (ret < 0)
		return ret;

	return smbus_write_word_data(s_ctx.fd, command, data);
}


static int I2CDEV_SMBReadBlock(unsigned int address, unsigned char command, unsigned char *data)
{
#if 0
    int ret = smbus_address(s_ctx.fd, address);
	if (ret < 0)
		return ret;

	// TBD: complete size check
    return smbus_read_block_data(s_ctx.fd, command, data);
#else
    return smbus_read_block_data_relax(s_ctx.fd, address, command, data);
#endif
}

static int I2CDEV_SMBWriteBlock(unsigned int address, unsigned char command, unsigned char *data, unsigned char len)
{
#if 0
    int ret = smbus_address(s_ctx.fd, address);
    if (ret < 0)
        return ret;
    // TBD: check total size, 32 max length is supported
    return smbus_write_block_data(s_ctx.fd, command, len, data);
#else
    return smbus_write_block_data_relax(s_ctx.fd, address, command, len, data);
#endif
}


static unsigned char I2CDEV_SMBGetLastReadPECFail()
{
	// TBD
	return 0;
}

static void I2CDEV_SMBEnablePEC(unsigned char state)
{
	smbus_pec(s_ctx.fd, !!state);
}

static int I2CDEV_SMBWrite(unsigned char start, unsigned char restart, unsigned char stop, unsigned char *data, unsigned int len)
{
	return -1;
}

static int I2CDEV_SMBRead(unsigned int len, unsigned char* data, unsigned char lastRead)
{
	return -1;
}


static int I2CDEV_SMBTransfer(struct SMBMsg *msgs, size_t count)
{
    struct i2c_msg *i2c_msgs = calloc(count, sizeof(struct i2c_msg));

    if (!i2c_msgs) {
        return -1;
    }

    for (unsigned i = 0; i < count; ++i) {
        i2c_msgs[i].addr  = msgs[i].addr>>1;
        i2c_msgs[i].flags = 0;
        i2c_msgs[i].len   = msgs[i].len;
        i2c_msgs[i].buf   = msgs[i].buf;

        if (msgs[i].flags & SMB_M_RD)
            i2c_msgs[i].flags |= I2C_M_RD;

        if (msgs[i].flags & SMB_M_NOSTART)
            i2c_msgs[i].flags |= I2C_M_NOSTART;

        if (msgs[i].flags & SMB_M_STOP)
            i2c_msgs[i].flags |= I2C_M_STOP;
    }

    struct i2c_rdwr_ioctl_data req = {
        .msgs = i2c_msgs,
        .nmsgs = count
    };

    int sts = xioctl(s_ctx.fd, I2C_RDWR, &req);

    free(i2c_msgs);

    return sts;
}

static unsigned int I2CDEV_SMBGetArbPEC()
{
	return -1;
}

static int I2CDEV_SMBTestAddressACK(unsigned int address)
{
	int ret = smbus_address(s_ctx.fd, address);
	if (ret)
		return ret;
	return smbus_write_quick(s_ctx.fd, 0);
}

static int I2CDEV_SMBTestCommandACK(unsigned int address, unsigned char command)
{
	int ret = smbus_address(s_ctx.fd, address);
	if (ret)
		return ret;
	return smbus_write_byte(s_ctx.fd, command);
}

static int I2CDEV_SMBTestCommandWrite(unsigned int address, unsigned char command)
{
	int result = 0;
	int sts;

	uint8_t outbuf[] = {
		command,  // Check command ACK == command exists
		3,        // Data byte #1 ACK  == byte writable
		0,        // Data byte #2 ACK  == word writable
		0,        // Skip
		0,        // Data byte #3 ACK  == block writable
		0,        // Data byte #4 ACK  >= block writable
	};

	const uint8_t size_checks[] = {
		1, // Command exists
		2, // Byte writable
		3, // Word writable
		5, // Block writable
		6, // >Block writable
	};

    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    for (unsigned i = 0; i < sizeof(size_checks); ++i) {
		msgs[0].addr = address>>1;
		msgs[0].flags = 0; /* I2C_M_TEN for 10 bits */
		msgs[0].len = size_checks[i];
		msgs[0].buf = outbuf;

		msgset[0].msgs = msgs;
		msgset[0].nmsgs = 1;

		sts = xioctl(s_ctx.fd, I2C_RDWR, &msgset);
		if (sts < 0)
			break;
		result++;
	}

	return result;
}


static void I2CDEV_SMBSetDebugLogFunc(void *logFunc)
{
	extLogFunc = logFunc;
}


static int I2CDEV_Open(const char *device_ops)
{
    // device_ops is a file name, like /dev/i2c-7
    const char* fileName = device_ops;
    if (!fileName) {
        fprintf(stderr, "I2CDEV: device file name is not provided\n");
        return -1;
    }
    return I2CDEV_SMBOpenDeviceI2c(fileName);
}


struct SMBDriver smbusb_i2cdev_driver = {
    .SMBOpen = I2CDEV_Open,
    .SMBClose = I2CDEV_SMBCloseDevice,
    .SMBReadByte = I2CDEV_SMBReadByte,
    .SMBSendByte = I2CDEV_SMBSendByte,
    .SMBWriteByte = I2CDEV_SMBWriteByte,
    .SMBReadWord = I2CDEV_SMBReadWord,
    .SMBWriteWord = I2CDEV_SMBWriteWord,
    .SMBReadBlock = I2CDEV_SMBReadBlock,
    .SMBWriteBlock = I2CDEV_SMBWriteBlock,
    .SMBGetLastReadPECFail = I2CDEV_SMBGetLastReadPECFail,
    .SMBEnablePEC = I2CDEV_SMBEnablePEC,
    .SMBWrite = I2CDEV_SMBWrite,
    .SMBRead = I2CDEV_SMBRead,
    .SMBTransfer = I2CDEV_SMBTransfer,
    .SMBGetArbPEC = I2CDEV_SMBGetArbPEC,
    .SMBTestAddressACK = I2CDEV_SMBTestAddressACK,
    .SMBTestCommandACK = I2CDEV_SMBTestCommandACK,
    .SMBTestCommandWrite = I2CDEV_SMBTestCommandWrite,
    .SMBSetDebugLogFunc = I2CDEV_SMBSetDebugLogFunc,
};
