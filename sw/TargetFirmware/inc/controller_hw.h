#ifndef CONTROLLER_HW_H
#define CONTROLLER_HW_H

#define IO_HIGH(p)   Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT,0,p)
#define IO_LOW(p)    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT,0,p)
#define IO_TOGGLE(p) Chip_GPIO_SetPinToggle(LPC_GPIO_PORT,0,p)
#define IO_READ(p)   Chip_GPIO_GetPinState(LPC_GPIO_PORT,0,p)
#define IO_OUTPUT(p) Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT,0,p)
#define IO_INPUT(p)  Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT,0,p)

static const uint8_t pin_vbat  = 23;
static const uint8_t pin_tx    = 13;
static const uint8_t pin_rx    = 17;
static const uint8_t pin_rf_tx = 11;
static const uint8_t pin_rf_rx = 10;
static const uint8_t pin_ir    = 15;

#endif
