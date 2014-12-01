#ifndef SCI_H
#define SCI_H


#define COMMAND_TYPE_INVALID 0
#define COMMAND_TYPE_READ    1
#define COMMAND_TYPE_WRITE   2

#define DEVICE_VOLTAGE       0
#define DEVICE_SPEED         1
#define DEVICE_RED_LED       2
#define DEVICE_BLUE_LED      3
#define DEVICE_HIT_ID        4

typedef struct
{
    uint8_t type;
    uint8_t address;
    uint8_t device;
    uint8_t data[2];
    uint8_t error;
} SciCommand;

SciCommand SciParse(uint8_t* buf);
uint8_t SciBytesRequired(uint8_t* buf);
void SciReadResponseCreate(uint8_t* buf, uint8_t data[2]);
void SciWriteResponseCreate(uint8_t* buf, uint8_t data);
void SciErrorResponseCreate(uint8_t* buf, uint8_t data);

#endif
