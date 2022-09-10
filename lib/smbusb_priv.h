#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

#include "libsmbusb.h"

struct SMBDriver
{
    int (*SMBOpen)(const char *device_ops);
    void (*SMBClose)(void);

    int (*SMBReadByte)(unsigned int address, unsigned char command);
    int (*SMBSendByte)(unsigned int address, unsigned char command);
    int (*SMBWriteByte)(unsigned int address, unsigned char command, unsigned char data);
    int (*SMBReadWord)(unsigned int address, unsigned char command);
    int (*SMBWriteWord)(unsigned int address, unsigned char command, unsigned int data);
    int (*SMBReadBlock)(unsigned int address, unsigned char command, unsigned char *data);
    int (*SMBWriteBlock)(unsigned int address, unsigned char command, unsigned char *data, unsigned char len);
    unsigned char (*SMBGetLastReadPECFail)(void);
    void (*SMBEnablePEC)(unsigned char state);
    int (*SMBWrite)(unsigned char start, unsigned char restart, unsigned char stop, unsigned char *data, unsigned int len);
    int (*SMBRead)(unsigned int len, unsigned char* data, unsigned char lastRead);
    int (*SMBTransfer)(struct SMBMsg *msgs, size_t count);
    unsigned int (*SMBGetArbPEC)(void);
    int (*SMBTestAddressACK)(unsigned int address);
    int (*SMBTestCommandACK)(unsigned int address, unsigned char command);
    int (*SMBTestCommandWrite)(unsigned int address, unsigned char command);
    void (*SMBSetDebugLogFunc)(void *logFunc);
};

struct OptDictItem
{
    const char *key;
    const char *val;
};

struct OptDict
{
    struct OptDictItem *items;
    size_t count;
    char *storage;
};

struct OptDict OptDictProcess(const char *driver_ops);
void OptDictRelease(struct OptDict *dict);

int GetNumBase(const char *numStr);
