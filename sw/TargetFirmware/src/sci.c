#include "global.h"
#include "sci.h"

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Local
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Addresses
static const uint8_t address_all          = '*';
static const uint8_t address_train        = '0';
static const uint8_t address_target_1     = '1';
static const uint8_t address_target_2     = '2';
static const uint8_t address_target_3     = '3';
static const uint8_t address_target_4     = '4';
static const uint8_t address_target_5     = '5';

// Devices
static const uint8_t device_train_voltage = 'V';
static const uint8_t device_train_speed   = 'S';
static const uint8_t device_target_rled   = 'R';
static const uint8_t device_target_bled   = 'B';
static const uint8_t device_hit_id        = 'I';

// Read
static const uint8_t read_command_start   = '{';
static const uint8_t read_command_stop    = '}';
typedef struct
{
    uint8_t start;
    uint8_t address;
    uint8_t device;
    uint8_t stop;
} ReadCommand;

static const uint8_t read_response_start  = '(';
static const uint8_t read_response_stop   = ')';
typedef struct
{
    uint8_t start;
    uint8_t data[2];
    uint8_t stop;
} ReadResponse;

// Write
static const uint8_t write_command_start  = '[';
static const uint8_t write_command_stop   = ']';
typedef struct
{
    uint8_t start;
    uint8_t address;
    uint8_t device;
    uint8_t data[2];
    uint8_t stop;
} WriteCommand;

static const uint8_t write_response_start = '<';
static const uint8_t write_response_stop  = '>';
typedef struct
{
    uint8_t start;
    uint8_t address;
    uint8_t stop;
} WriteResponse;

// Errors
static const uint8_t error_address        = 'A';
static const uint8_t error_device         = 'T';
static const uint8_t error_read_only      = 'R';
static const uint8_t error_write_only     = 'W';

static const uint8_t error_response_start = 'W';
static const uint8_t error_response_stop  = 'F';
typedef struct
{
    uint8_t start;
    uint8_t error;
    uint8_t stop;
} ErrorResponse;

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Command handling
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
SciCommand SciParse(uint8_t* buf)
{
    // TODO
}

uint8_t SciBytesRequired(uint8_t* buf)
{
    uint8_t start_byte = buf[0];
    uint8_t bytes_required = 0;

    if (start_byte == read_command_start)
    {
        bytes_required = sizeof(ReadCommand);
    }
    else if (start_byte == write_command_start)
    {
        bytes_required = sizeof(WriteCommand);
    }
    return bytes_required;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Response handling
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void SciReadResponseCreate(uint8_t* buf, uint8_t data[2])
{
    // TODO
}

void SciWriteResponseCreate(uint8_t* buf, uint8_t data)
{
    // TODO
}

void SciErrorResponseCreate(uint8_t* buf, uint8_t data)
{
    // TODO
}

