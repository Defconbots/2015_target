#ifndef SCI_H
#define SCI_H

// Command types
#define COMMAND_TYPE_INVALID 0
#define COMMAND_TYPE_READ    1
#define COMMAND_TYPE_WRITE   2

// Device types
#define DEVICE_VOLTAGE       'V'
#define DEVICE_SPEED         'S'
#define DEVICE_RED_LED       'R'
#define DEVICE_BLUE_LED      'B'
#define DEVICE_HIT_ID        'I'

// Addresses
#define ADDRESS_ALL          '*'
#define ADDRESS_TRAIN        '0'
#define ADDRESS_TARGET_1     '1'
#define ADDRESS_TARGET_2     '2'
#define ADDRESS_TARGET_3     '3'
#define ADDRESS_TARGET_4     '4'
#define ADDRESS_TARGET_5     '5'

// Train Speeds
#define SPEED_SLOW_ASTERN    "R1"
#define SPEED_STOP           "ST"
#define SPEED_HALF_AHEAD     "F1"
#define SPEED_FULL_AHEAD     "F2"

typedef struct
{
    uint8_t type;
    uint8_t address;
    uint8_t device;
    uint8_t data[2];
    uint8_t error;
} SciCommand;

SciCommand SciParse(uint8_t* buf);
void SciReadResponseCreate(uint8_t* buf, uint8_t address, uint8_t data[2]);
void SciWriteResponseCreate(uint8_t* buf, uint8_t address);
void SciErrorResponseCreate(uint8_t* buf, uint8_t data);

#endif
