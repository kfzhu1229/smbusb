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

#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(1000*x)
#endif

#define ERR_DEVICE_OPEN	-1
#define ERR_ALREADY_OPEN -5
#define ERR_CLAIM_INTERFACE -10
#define ERR_FIRMWARE_DOWNLOAD -11
#define INIT_RETRY -20

#define SMB_INTERFACE_ID 0x99
#define SMB_FIRMWARE_VERSION 0x98

#define SMB_ENABLE_PEC 0x5

// Standard SMB protocol convenience commands

#define SMB_READ_BYTE 0x10
#define SMB_WRITE_BYTE 0x11
#define SMB_SEND_BYTE 0x12

#define SMB_READ_WORD 0x20
#define SMB_WRITE_WORD 0x21

#define SMB_READ_BLOCK 0x30
#define SMB_WRITE_BLOCK 0x32

#define SMB_GET_CLEAR_PEC_FAIL 0x54

// Arbitrary SMB(/I2C) operations
#define SMB_WRITE 0x50			//smb_addr = length, smb_cmd = write_cmd
#define SMB_WRITE_CMD_START_FIRST 0x1
#define SMB_WRITE_CMD_RESTART_FIRST 0x2
#define SMB_WRITE_CMD_STOP_AFTER 0x4

#define SMB_READ 0x51			// smb_addr = length, smb_cmd = read_cmd
#define SMB_READ_CMD_FIRST_READ 0x1
#define SMB_READ_CMD_LAST_READ 0x2	// last read block, handles LASTRD, STOP

#define SMB_GET_MRQ_PECS	0x55

#define SMB_STOP 0x60
                                 
// SMB Hacking and Discovery

#define SMB_TEST_ADDRESS_ACK 0x90
#define SMB_TEST_COMMAND_ACK 0x91
#define SMB_TEST_COMMAND_WRITE 0x92

/**
 * Open specific device URI with driver-specific options in driver-specific way
 *
 * Drivers:
 *  * fx2lp
 *  * i2cdev
 *
 * fx2lp URI:
 *    fx2lp://<<vid=VID,pid=PID>|<bus=BUS,addr=ADDR>>
 *   where:
 *      VID, PID, BUS and ADDR numeric values in HEX (0xXX, XXh), OCT (0XX), BIN (0bXX) or DEC (XX) forms
 *      VID and PID must be paired together and not mixed with BUS and ADDR and otherwise
 *   example:
 *      fx2lp://vid=0x04b4,pid=0x8613
 *
 * i2cdev URI:
 *     i2cdev://<DEV>
 *     i2c://<DEV>
 *   where:
 *     DEV is a full or relative path to the I2C device exported by the i2c-dev module
 *   example:
 *     i2cdev:///dev/i2c-7
 *     i2c:///dev/i2c-7
 *
 * @param device_uri  URI for opening
 * @return
 */
extern int SMBOpenDevice(const char *device_uri);
extern void SMBCloseDevice(void);

//extern unsigned int SMBInterfaceID(void);

extern int SMBSendByte(unsigned int address, unsigned char command);
extern int SMBReadByte(unsigned int address, unsigned char command);
extern int SMBWriteByte(unsigned int address, unsigned char command, unsigned char data);
extern int SMBReadWord(unsigned int address, unsigned char command);
extern int SMBWriteWord(unsigned int address, unsigned char command, unsigned int data);
extern int SMBReadBlock(unsigned int address, unsigned char command, unsigned char *data);
extern int SMBWriteBlock(unsigned int address, unsigned char command, unsigned char *data, unsigned char len);

extern void SMBEnablePEC(unsigned char state);
extern unsigned char SMBGetLastReadPECFail(void);

extern int SMBWrite(unsigned char start, unsigned char restart, unsigned char stop, unsigned char *data, unsigned int len);
extern int SMBRead(unsigned int len, unsigned char* data, unsigned char lastRead);
extern unsigned int SMBGetArbPEC(void);

extern int SMBTestAddressACK(unsigned int address);
extern int SMBTestCommandACK(unsigned int address, unsigned char command);
extern int SMBTestCommandWrite(unsigned int address, unsigned char command);

extern void SMBSetDebugLogFunc(void *logFunc);

struct SMBMsg {
    uint16_t addr;
    uint16_t flags;
#define SMB_M_RD       0x0001
#define SMB_M_NOSTART  0x4000
#define SMB_M_STOP     0x8000
    uint16_t len;
    uint8_t *buf;
};

extern int SMBTransfer(struct SMBMsg *msgs, size_t count);

