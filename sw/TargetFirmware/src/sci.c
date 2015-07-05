#include "global.h"
#include "sci.h"
#include <string.h>

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Local
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Devices
#define DEVICE_TRAIN_VOLTAGE 'V'
#define DEVICE_TRAIN_SPEED   'S'
#define DEVICE_TARGET_RLED   'R'
#define DEVICE_TARGET_BLED   'B'
#define DEVICE_TARGET_HIT_ID 'I'

// Read
#define READ_COMMAND_START   '{'
#define READ_COMMAND_STOP    '}'
typedef struct
{
    uint8_t start;
    uint8_t address;
    uint8_t device;
    uint8_t stop;
} ReadCommand;

#define READ_RESPONSE_START  '('
#define READ_RESPONSE_STOP   ')'
typedef struct
{
    uint8_t start;
    uint8_t address;
    uint8_t device;
    uint8_t data[2];
    uint8_t stop;
} ReadResponse;

// Write
#define WRITE_COMMAND_START  '['
#define WRITE_COMMAND_STOP   ']'
typedef struct
{
    uint8_t start;
    uint8_t address;
    uint8_t device;
    uint8_t data[2];
    uint8_t stop;
} WriteCommand;

#define WRITE_RESPONSE_START '<'
#define WRITE_RESPONSE_STOP  '>'
typedef struct
{
    uint8_t start;
    uint8_t address;
    uint8_t device;
    uint8_t stop;
} WriteResponse;

// Errors
#define ERROR_ADDRESS        'A'
#define ERROR_DEVICE         'T'
#define ERROR_MALFORMED      'M'

#define ERROR_RESPONSE_START 'W'
#define ERROR_RESPONSE_STOP  'F'
typedef struct
{
    uint8_t address;
    uint8_t start;
    uint8_t error;
    uint8_t stop;
} ErrorResponse;

uint8_t SciBytesRequired(uint8_t start_byte);
uint8_t SciAddressVerify(uint8_t* buf);
uint8_t SciStartStopVerify(uint8_t* buf);
uint8_t SciDeviceVerify(uint8_t* buf);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Command handling
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
SciCommand SciParse(uint8_t* buf)
{
    SciCommand ret = {COMMAND_TYPE_INVALID,0,0,{0},0};
    uint8_t start_byte = buf[0];
    uint8_t len = (uint8_t)strlen((char*)&buf[0]);
    uint8_t bytes_required = SciBytesRequired(start_byte);
    uint8_t address_valid = SciAddressVerify(&buf[0]);
    uint8_t start_stop_valid = SciStartStopVerify(&buf[0]);
    uint8_t device_valid = SciDeviceVerify(&buf[0]);

    if (len < bytes_required || !start_stop_valid)
    {
        ret.error = ERROR_MALFORMED;
    }
    else if (!device_valid)
    {
        ret.error = ERROR_DEVICE;
    }
    else if (!address_valid)
    {
        ret.error = ERROR_ADDRESS;
    }
    else if (start_byte == READ_COMMAND_START)
    {
        ReadCommand* command = (ReadCommand*)buf;
        ret.type = COMMAND_TYPE_READ;
        ret.address = command->address;
        ret.device = command->device;
    }
    else if (start_byte == WRITE_COMMAND_START)
    {
        WriteCommand* command = (WriteCommand*)buf;
        ret.type = COMMAND_TYPE_WRITE;
        ret.address = command->address;
        ret.device = command->device;
        memcpy(ret.data,command->data,sizeof(ret.data));
    }
    return ret;
}

uint8_t SciBytesRequired(uint8_t start_byte)
{
    uint8_t bytes_required = 0;
    if (start_byte == READ_COMMAND_START)
    {
        bytes_required = sizeof(ReadCommand);
    }
    else if (start_byte == WRITE_COMMAND_START)
    {
        bytes_required = sizeof(WriteCommand);
    }
    return bytes_required;
}

uint8_t SciAddressVerify(uint8_t* buf)
{
    uint8_t address = buf[1];
    switch(address)
    {
        case ADDRESS_ALL:
        case ADDRESS_TRAIN:
        case ADDRESS_TARGET_1:
        case ADDRESS_TARGET_2:
        case ADDRESS_TARGET_3:
        case ADDRESS_TARGET_4:
        case ADDRESS_TARGET_5:
        {
            return TRUE;
        }
    }
    return false;
}

uint8_t SciStartStopVerify(uint8_t* buf)
{
    uint8_t start_byte = buf[0];
    if ((start_byte == READ_COMMAND_START) &&
       (((ReadCommand*)buf)->stop != READ_COMMAND_STOP))
    {
        return FALSE;
    }
    else if ((start_byte == WRITE_COMMAND_START) &&
            (((WriteCommand*)buf)->stop != WRITE_COMMAND_STOP))
    {
        return FALSE;
    }
    return TRUE;
}

uint8_t SciDeviceVerify(uint8_t* buf)
{
    uint8_t device = buf[2];
    switch(device)
    {
        case DEVICE_TRAIN_VOLTAGE:
        case DEVICE_TRAIN_SPEED:
        case DEVICE_TARGET_RLED:
        case DEVICE_TARGET_BLED:
        case DEVICE_TARGET_HIT_ID:
        {
            return TRUE;
        }
    }
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Response handling
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void SciReadResponseCreate(uint8_t* buf, uint8_t device, uint8_t data[2])
{
    ReadResponse* response = (ReadResponse*)buf;
    response->start = READ_RESPONSE_START;
    response->address = MY_ADDRESS;
    response->device = device;
    memcpy(response->data,data,sizeof(response->data));
    response->stop = READ_RESPONSE_STOP;
}

void SciWriteResponseCreate(uint8_t* buf, uint8_t device)
{
    WriteResponse* response = (WriteResponse*)buf;
    response->start = WRITE_RESPONSE_START;
    response->address = MY_ADDRESS;
    response->device = device;
    response->stop = WRITE_RESPONSE_STOP;
}

void SciErrorResponseCreate(uint8_t* buf, uint8_t data)
{
    ErrorResponse* response = (ErrorResponse*)buf;
    response->address = MY_ADDRESS;
    response->start = ERROR_RESPONSE_START;
    response->error = data;
    response->stop = ERROR_RESPONSE_STOP;
}

