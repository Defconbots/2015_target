#ifndef TARGET_HW_H
#define TARGET_HW_H

#define IO_HIGH(p)   Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT,0,p)
#define IO_LOW(p)    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT,0,p)
#define IO_TOGGLE(p) Chip_GPIO_SetPinToggle(LPC_GPIO_PORT,0,p)
#define IO_READ(p)   Chip_GPIO_GetPinState(LPC_GPIO_PORT,0,p)
#define IO_OUTPUT(p) Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT,0,p)
#define IO_INPUT(p)  Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT,0,p)

static const uint8_t pin_rx        = 17;
static const uint8_t pin_tx        = 13;
static const uint8_t pin_tx_idle   = 10;
static const uint8_t pin_blue_led  = 14;
static const uint8_t pin_blue_idle = 23;
static const uint8_t pin_red_led   = 0;
static const uint8_t pin_red_idle  = 12;
static const uint8_t pin_laser_rx = 15;

#endif
