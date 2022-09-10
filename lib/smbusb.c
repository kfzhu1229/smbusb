#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "libsmbusb.h"
#include "smbusb_priv.h"

extern struct SMBDriver smbusb_fx2lp_driver;
extern struct SMBDriver smbusb_i2cdev_driver;

static struct SMBDriver *s_drv = NULL;

int SMBOpenDevice(const char *device_uri)
{
    int status = 0;
    const char *device_ops = NULL;

    if (s_drv) {
        fprintf(stderr, "Driver/Device already opened\n");
        return -1;
    }

#ifdef USE_FX2LP_PROGRAMMER
    if (!strncmp(device_uri, "fx2lp://", 8)) {
        s_drv = &smbusb_fx2lp_driver;
        device_ops = device_uri + 8; // skip "fx2lp://"
    }
#endif

#ifdef USE_I2CDEV_PROGRAMMER
    if (!strncmp(device_uri, "i2cdev://", 9)) {
        s_drv = &smbusb_i2cdev_driver;
        device_ops = device_uri + 9; // skip "i2cdev://"
    } else if (!strncmp(device_uri, "i2c://", 6)) {
        s_drv = &smbusb_i2cdev_driver;
        device_ops = device_uri + 6; // skip "i2c://"
    }
#endif

    if (!s_drv) {
        fprintf(stderr, "Unsupported device URI: \"%s\"\n", device_uri);
        return -1;
    }

    status = s_drv->SMBOpen(device_ops);
    if (status < 0)
        s_drv = NULL;

    return status;
}


void SMBCloseDevice(void)
{
    if (!s_drv)
        return;
    s_drv->SMBClose();
}

int SMBSendByte(unsigned int address, unsigned char command)
{
    return !s_drv ? -1 : s_drv->SMBSendByte(address, command);
}

int SMBReadByte(unsigned int address, unsigned char command)
{
    return !s_drv ? -1 : s_drv->SMBReadByte(address, command);
}

int SMBWriteByte(unsigned int address, unsigned char command, unsigned char data)
{
    return !s_drv ? -1 : s_drv->SMBWriteByte(address, command, data);
}

int SMBReadWord(unsigned int address, unsigned char command)
{
    return !s_drv ? -1 : s_drv->SMBReadWord(address, command);
}

int SMBWriteWord(unsigned int address, unsigned char command, unsigned int data)
{
    return !s_drv ? -1 : s_drv->SMBWriteWord(address, command, data);
}

int SMBReadBlock(unsigned int address, unsigned char command, unsigned char *data)
{
    return !s_drv ? -1 : s_drv->SMBReadBlock(address, command, data);
}

int SMBWriteBlock(unsigned int address, unsigned char command, unsigned char *data, unsigned char len)
{
    return !s_drv ? -1 : s_drv->SMBWriteBlock(address, command, data, len);
}

void SMBEnablePEC(unsigned char state)
{
    !s_drv ? (void)0 : s_drv->SMBEnablePEC(state);
}

unsigned char SMBGetLastReadPECFail(void)
{
    return !s_drv ? 0x00 : s_drv->SMBGetLastReadPECFail();
}

int SMBWrite(unsigned char start, unsigned char restart, unsigned char stop, unsigned char *data, unsigned int len)
{
    return !s_drv ? -1 : s_drv->SMBWrite(start, restart, stop, data, len);
}

int SMBRead(unsigned int len, unsigned char* data, unsigned char lastRead)
{
    return !s_drv ? -1 : s_drv->SMBRead(len, data, lastRead);
}

unsigned int SMBGetArbPEC(void)
{
    return !s_drv ? 0 : s_drv->SMBGetArbPEC();
}

int SMBTestAddressACK(unsigned int address)
{
    return !s_drv ? -1 : s_drv->SMBTestAddressACK(address);
}

int SMBTestCommandACK(unsigned int address, unsigned char command)
{
    return !s_drv ? -1 : s_drv->SMBTestCommandACK(address, command);
}

int SMBTestCommandWrite(unsigned int address, unsigned char command)
{
    return !s_drv ? -1 : s_drv->SMBTestCommandWrite(address, command);
}

void SMBSetDebugLogFunc(void *logFunc)
{
    !s_drv ? (void)0 : s_drv->SMBSetDebugLogFunc(logFunc);
}

int SMBTransfer(struct SMBMsg *msgs, size_t count)
{
    return !s_drv ? -1 : s_drv->SMBTransfer(msgs, count);
}

struct OptDict OptDictProcess(const char *driver_ops)
{
    struct OptDict result = {};

    result.storage = strdup(driver_ops);
    if (!result.storage)
        return result;

    char *saveptr1, *saveptr2;
    char *token, *subtoken, *str1, *str2;

    for (str1 = result.storage; ;str1 = NULL) {
        token = strtok_r(str1, ",", &saveptr1);
        if (!token)
            break;

        result.count++;
        struct OptDictItem *items = realloc(result.items, sizeof(*result.items) * result.count);
        if (!items)
            goto cleanup;
        result.items = items;

        struct OptDictItem *item = &result.items[result.count-1];
        memset(item, 0, sizeof(*item));

        for (str2 = token; ; str2 = NULL) {
            subtoken = strtok_r(str2, "=", &saveptr2);
            if (!subtoken)
                break;

            if (!item->key) {
                item->key = subtoken;
            } else {
                if (!item->val)
                    item->val = subtoken;
                else
                    subtoken[-1] = '=';
            }
        }
    }

    return result;

cleanup:
    free(result.storage);
    return (struct OptDict){};
}

void OptDictRelease(struct OptDict *dict)
{
    free(dict->items);
    free(dict->storage);
    dict->items = NULL;
    dict->storage = NULL;
    dict->count = 0;
}

int GetNumBase(const char *numStr)
{
    size_t len = strlen(numStr);
    if (!len)
        return 10;

    // 0x0
    if (len >= 3 && numStr[0] == 0 && (numStr[1] == 'x' || numStr[1] == 'X'))
        return 16;

    // 0b0
    if (len >= 3 && numStr[0] == 0 && (numStr[1] == 'b' || numStr[1] == 'B'))
        return 2;

    // 00
    if (len >= 2 && (numStr[0] == 0 && numStr[len - 1] != 'h'))
        return 8;

    // 00h
    if (len >= 2 && numStr[len-1] == 'h')
        return 16;

    return 10;
}
