#include "global.h"
#include "target.h"
#include "target_hw.h"
#include "sci.h"
#include "train.h"
#include "schedule.h"
#include "delay.h"
#include <string.h>

#if defined (TARGET)
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Local
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void    TrainService(void);
void    LaserService(void);
void    HardwareInit(void);
void    UartInit(void);
void    TrainWrite(uint8_t* buf);
uint8_t TrainRead(uint8_t* buf, uint8_t len);
uint8_t TrainDataAvailable(void);
uint8_t LaserDataAvailable(void);
uint8_t LaserRead(uint8_t* buf, uint8_t len);
void    LedRedRead(uint8_t data[2]);
void    LedRedWrite(uint8_t data[2]);
void    LedBlueRead(uint8_t data[2]);
void    LedBlueWrite(uint8_t data[2]);
void    HitIdRead(uint8_t data[2]);
void    HitIdWrite(uint8_t data[2]);

#define RING_BUF_SIZE 32
#define BUF_SIZE      10

#define TRAIN_UART          LPC_USART0
#define TRAIN_UART_IRQ      UART0_IRQn
#define TRAIN_UART_HANDLER  UART0_IRQHandler
STATIC  RINGBUFF_T          train_rx_ring;
static  uint8_t             train_rx_buf[RING_BUF_SIZE];

#define LASER_UART          LPC_USART1
#define LASER_UART_IRQ      UART1_IRQn
#define LASER_UART_HANDLER  UART1_IRQHandler
STATIC  RINGBUFF_T          laser_rx_ring;
static  uint8_t             laser_rx_buf[RING_BUF_SIZE];

const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

typedef struct
{
    uint8_t type;
    uint8_t device;
    void (*HandlerFn)(uint8_t*);
} CommandRoute;

static uint8_t  hit_id = 0;
static uint16_t red_led = 0;
static uint16_t blue_led = 0;

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Entry
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
inline void Run(void)
{
    ScheduleInit();
    HardwareInit();
    UartInit();
    for(;;)
    {
        LaserService();
        TrainService();
    }
}

void LaserService(void)
{
    CommandRoute routes[] =
    {
        {COMMAND_TYPE_READ,  DEVICE_HIT_ID, HitIdRead},
        {COMMAND_TYPE_WRITE, DEVICE_HIT_ID, HitIdWrite}
    };

    if (LaserDataAvailable())
    {
        // Wait a little for the whole packet to arrive
        Delay(3);
        uint8_t buf[BUF_SIZE] = {0};
        LaserRead(&buf[0],BUF_SIZE);
        SciCommand command = SciParse(&buf[0]);
        if (command.address == ADDRESS_ALL && command.type != COMMAND_TYPE_INVALID)
        {
            uint8_t i = 0;
            for (i = 0;i < (sizeof(routes)/sizeof(CommandRoute));i++)
            {
                if (command.type   == routes[i].type &&
                    command.device == routes[i].device)
                {
                    routes[i].HandlerFn(command.data);
                    return;
                }
            }
        }
    }
}

void TrainService(void)
{
    CommandRoute routes[] =
    {
        {COMMAND_TYPE_READ,  DEVICE_RED_LED,  LedRedRead},
        {COMMAND_TYPE_WRITE, DEVICE_RED_LED,  LedRedWrite},
        {COMMAND_TYPE_READ,  DEVICE_BLUE_LED, LedBlueRead},
        {COMMAND_TYPE_WRITE, DEVICE_BLUE_LED, LedBlueWrite}
    };

    if (TrainDataAvailable())
    {
        // Wait a little for the whole packet to arrive
        Delay(3);
        uint8_t buf[BUF_SIZE] = {0};
        TrainRead(&buf[0],BUF_SIZE);
        SciCommand command = SciParse(&buf[0]);
        if (command.address == MY_ADDRESS)
        {
            if (command.type == COMMAND_TYPE_INVALID)
            {
                SciErrorResponseCreate(&buf[0],command.error);
                TrainWrite(&buf[0]);
            }
            else
            {
                uint8_t i = 0;
                for (i = 0;i < (sizeof(routes)/sizeof(CommandRoute));i++)
                {
                    if (command.type   == routes[i].type &&
                        command.device == routes[i].device)
                    {
                        routes[i].HandlerFn(command.data);
                        return;
                    }
                }
            }
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Init
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void HardwareInit(void)
{
    // TODO: init LEDs
}

void UartInit(void)
{
    // Enable switch matrix clock
    Chip_SWM_Init();
    // Disable fixed functions for the tx/rx pins
    Chip_SWM_DisableFixedPin(SWM_FIXED_ADC9);  // RX
    Chip_SWM_DisableFixedPin(SWM_FIXED_ADC10); // TX

    // Uart div clock by 1
    Chip_Clock_SetUARTClockDiv(1);

    // Assign pin locations for tx/rx/rf_tx/rf_rx
    Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, pin_tx); // TX
    Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, pin_rx); // RX
    Chip_SWM_MovablePinAssign(SWM_U1_RXD_I, pin_laser_rx); // LASER_RX

    // Disable switch matrix clock (saves power)
    Chip_SWM_Deinit();

    // Init the Uart hardware
    Chip_UART_Init(TRAIN_UART);
    Chip_UART_ConfigData(TRAIN_UART,UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
    Chip_UART_SetBaud(TRAIN_UART,115200);
    Chip_UART_Enable(TRAIN_UART);

    Chip_UART_Init(LASER_UART);
    Chip_UART_ConfigData(LASER_UART,UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
    Chip_UART_SetBaud(LASER_UART,9600);
    Chip_UART_Enable(LASER_UART);

    // Init ring buffers
    RingBuffer_Init(&train_rx_ring,train_rx_buf,1,RING_BUF_SIZE);
    RingBuffer_Init(&laser_rx_ring,laser_rx_buf,1,RING_BUF_SIZE);

    // Enable interrupts
    Chip_UART_IntEnable(TRAIN_UART,UART_INTEN_RXRDY);
    NVIC_EnableIRQ(TRAIN_UART_IRQ);
    Chip_UART_IntEnable(LASER_UART,UART_INTEN_RXRDY);
    NVIC_EnableIRQ(LASER_UART_IRQ);
}

// Hook into Uart interrupts and redirect them to ring buffers
void TRAIN_UART_HANDLER(void)
{
    Chip_UART_RXIntHandlerRB(TRAIN_UART, &train_rx_ring);
}

void LASER_UART_HANDLER(void)
{
    Chip_UART_RXIntHandlerRB(LASER_UART, &laser_rx_ring);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Communication
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void TrainWrite(uint8_t* buf)
{
    // All targets share a bus so we can't leave tx enabled
    Chip_UART_TXEnable(TRAIN_UART);
    Delay(3);
    Chip_UART_SendBlocking(TRAIN_UART,&buf,strlen((char*)buf));
    Chip_UART_TXDisable(TRAIN_UART);
}

uint8_t TrainRead(uint8_t* buf, uint8_t len)
{
    return Chip_UART_ReadRB(TRAIN_UART,&train_rx_ring,&buf,len);
}

uint8_t TrainDataAvailable(void)
{
    return (uint8_t)RingBuffer_GetCount(&train_rx_ring);
}

uint8_t LaserRead(uint8_t* buf, uint8_t len)
{
    return Chip_UART_ReadRB(LASER_UART,&laser_rx_ring,&buf,len);
}

uint8_t LaserDataAvailable(void)
{
    return (uint8_t)RingBuffer_GetCount(&laser_rx_ring);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// TODO there might be some endianess problems here with the casts
void LedRedRead(uint8_t data[2])
{
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0],(uint8_t*)&red_led);
    TrainWrite(&buf[0]);
}

void LedRedWrite(uint8_t data[2])
{
    red_led = *(uint16_t*)data;
    // TODO: actually change LED hw status here
    uint8_t buf[BUF_SIZE] = {0};
    SciWriteResponseCreate(&buf[0],MY_ADDRESS);
    TrainWrite(&buf[0]);
}

void LedBlueRead(uint8_t data[2])
{
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0],(uint8_t*)&blue_led);
    TrainWrite(&buf[0]);
}

void LedBlueWrite(uint8_t data[2])
{
    blue_led = *(uint16_t*)data;;
    // TODO: actually change LED hw status here
    uint8_t buf[BUF_SIZE] = {0};
    SciWriteResponseCreate(&buf[0],MY_ADDRESS);
    TrainWrite(&buf[0]);
}

void HitIdRead(uint8_t data[2])
{
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0],(uint8_t*)&hit_id);
    TrainWrite(&buf[0]);
}

void HitIdWrite(uint8_t data[2])
{
    blue_led = *(uint16_t*)data;;
    // TODO: actually change LED hw status here
    uint8_t buf[BUF_SIZE] = {0};
    SciWriteResponseCreate(&buf[0],MY_ADDRESS);
    TrainWrite(&buf[0]);
}

#endif
