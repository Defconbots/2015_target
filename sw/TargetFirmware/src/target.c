#include "global.h"
#include "target.h"
#include "target_hw.h"
#include "sci.h"
#include "schedule.h"
#include "delay.h"
#include <string.h>

#if defined (TARGET)
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Local
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#define BUF_SIZE            16

#define LED_JUICY_ALPHA     (0.03)
#define LED_ARRAY_SIZE      10

#define TRAIN_UART          LPC_USART0
#define TRAIN_UART_IRQ      UART0_IRQn
#define TRAIN_UART_HANDLER  UART0_IRQHandler
#define TRAIN_UART_BAUD     57600
STATIC  RINGBUFF_T          train_rx_ring;
static  uint8_t             train_rx_buf[BUF_SIZE];

#define LASER_UART          LPC_USART1
#define LASER_UART_IRQ      UART1_IRQn
#define LASER_UART_HANDLER  UART1_IRQHandler
#define LASER_UART_BAUD     4800
STATIC  RINGBUFF_T          laser_rx_ring;
static  uint8_t             laser_rx_buf[BUF_SIZE];

#define UART_CONF_8N1       (UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1)

const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

typedef void (*HandlerFn)(uint8_t*);
typedef struct
{
    uint8_t type;
    uint8_t device;
    HandlerFn handler;
} CommandRoute;

// Pwm settings (LEDs)
#define PWM_RATE            1000
#define PWM_INDEX_RED       1
#define PWM_INDEX_BLUE      2

enum {ALL_YOUR_BASE,ARE_BELONG,TO_US};
#define YOU_ARE_ON_THE_WAY  0x41
#define TO_DESTRUCTION      0x61
static uint8_t  _the_bomb = 0;
static uint16_t _hit_id   = 0;
static uint8_t  _led_red  = 0;
static uint8_t  _led_blue = 0;


static void      LaserService(void);
static void      TrainService(void);
static HandlerFn HandlerSearch(SciCommand command, CommandRoute* routes, uint8_t len);
static void      PwmInit(void);
static void      UartInit(void);
static void      TrainTxBusIdle(void);
static void      TrainTxBusActive(void);
static void      TrainWrite(uint8_t* buf);
static uint8_t   TrainRead(uint8_t* buf, uint8_t len);
static uint8_t   TrainDataAvailable(void);
static uint8_t   LaserDataAvailable(void);
static uint8_t   LaserRead(uint8_t* buf, uint8_t len);
static void      LedManageRed(void);
static void      LedManageBlue(void);
static void      LedSetRed(uint8_t brightness);
static void      LedSetBlue(uint8_t brightness);
static void      LedReadRed(uint8_t data[2]);
static void      LedWriteRed(uint8_t data[2]);
static void      LedReadBlue(uint8_t data[2]);
static void      LedWriteBlue(uint8_t data[2]);
static void      HitIdRead(uint8_t data[2]);
static void      HitIdWrite(uint8_t data[2]);
static void      HitIdWriteSilent(uint8_t data[2]);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Entry
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
inline void Run(void)
{
    ScheduleInit();
    PwmInit();
    UartInit();

#ifdef TARGET_TEST_SERIAL
    TrainWrite("Train link up\n");
    while(1)
    {
        if (LaserDataAvailable())
        {
            uint8_t buf[BUF_SIZE] = {0};
            LaserRead(&buf[0],BUF_SIZE);
            TrainWrite(&buf[0]);
        }
        if (TrainDataAvailable())
        {
            uint8_t buf[BUF_SIZE] = {0};
            TrainRead(&buf[0],BUF_SIZE);
            Delay(5);
            TrainWrite(&buf[0]);
        }
    }
#endif

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
        {COMMAND_TYPE_WRITE, DEVICE_HIT_ID, HitIdWriteSilent}
    };
    uint8_t route_len = sizeof(routes) / sizeof(CommandRoute);

    if (LaserDataAvailable())
    {
        // Wait a little for the whole packet to arrive
        Delay(20);
        uint8_t buf[BUF_SIZE] = {0};
        LaserRead(&buf[0],BUF_SIZE);
        SciCommand command = SciParse(&buf[0]);
        if (command.type != COMMAND_TYPE_INVALID)
        {
            HandlerFn fn = HandlerSearch(command,&routes[0],route_len);
            if (fn != NULL)
            {
                _the_bomb = command.address == ADDRESS_ALL ? ALL_YOUR_BASE :
                            command.address == MY_ADDRESS  ? ARE_BELONG:
                                                             TO_US;
                fn(command.data);
            }
        }
    }
}

void TrainService(void)
{
    CommandRoute routes[] =
    {
        {COMMAND_TYPE_READ,  DEVICE_RED_LED,  LedReadRed},
        {COMMAND_TYPE_WRITE, DEVICE_RED_LED,  LedWriteRed},
        {COMMAND_TYPE_READ,  DEVICE_BLUE_LED, LedReadBlue},
        {COMMAND_TYPE_WRITE, DEVICE_BLUE_LED, LedWriteBlue},
        {COMMAND_TYPE_READ,  DEVICE_HIT_ID,   HitIdRead},
        {COMMAND_TYPE_WRITE, DEVICE_HIT_ID,   HitIdWrite}
    };
    uint8_t route_len = sizeof(routes) / sizeof(CommandRoute);

    if (TrainDataAvailable())
    {
        TrainTxBusIdle();
        // Wait a little for the whole packet to arrive
        Delay(5);
        uint8_t buf[BUF_SIZE] = {0};
        TrainRead(&buf[0],BUF_SIZE);
        SciCommand command = SciParse(&buf[0]);
        if (command.type != COMMAND_TYPE_INVALID && command.address == MY_ADDRESS)
        {
            HandlerFn fn = HandlerSearch(command,&routes[0],route_len);
            if (fn != NULL)
            {
                fn(command.data);
            }
        }
    }
}

HandlerFn HandlerSearch(SciCommand command, CommandRoute* routes, uint8_t len)
{
    for (uint8_t i = 0;i < len;i++)
    {
        if (command.type   == routes[i].type &&
            command.device == routes[i].device)
        {
            return routes[i].handler;
        }
    }
    return NULL;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Init
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void PwmInit(void)
{
    LedSetBlue(0);
    LedSetRed(0);
    Chip_SCTPWM_Init(LPC_SCT);
    Chip_SCTPWM_SetRate(LPC_SCT, PWM_RATE);
    Chip_SCTPWM_SetOutPin(LPC_SCT, PWM_INDEX_RED, 0);
    Chip_SCTPWM_SetDutyCycle(LPC_SCT, PWM_INDEX_RED, 0);
    Chip_SCTPWM_SetOutPin(LPC_SCT, PWM_INDEX_BLUE, 1);
    Chip_SCTPWM_SetDutyCycle(LPC_SCT, PWM_INDEX_BLUE, 0);
    Chip_SCTPWM_Start(LPC_SCT);
}

void UartInit(void)
{
    // Enable switch matrix clock
    Chip_SWM_Init();
    Chip_IOCON_Init();

    Chip_Clock_SetUARTClockDiv(1);
    Chip_Clock_SetUSARTNBaseClockRate((TRAIN_UART_BAUD * 16), true);

    // TX is on a bus so we redirect it to a spare IO (PIO0_10) when not in use
    // Float the tx pin
    Chip_IOCON_PinSetMode(LPC_IOCON,IOCON_PIO13,PIN_MODE_INACTIVE);
    Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, pin_tx_idle); // Train TX
    Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, pin_rx); // Train RX
    Chip_SWM_MovablePinAssign(SWM_U1_RXD_I, pin_laser_rx); // LASER_RX

    // Disable switch matrix clock (saves power)
    Chip_SWM_Deinit();
    Chip_IOCON_Deinit();

    // Init the Uart hardware
    Chip_UART_Init(TRAIN_UART);
    Chip_UART_ConfigData(TRAIN_UART,UART_CONF_8N1);
    Chip_UART_SetBaud(TRAIN_UART,TRAIN_UART_BAUD);
    Chip_UART_Enable(TRAIN_UART);
    RingBuffer_Init(&train_rx_ring,train_rx_buf,1,BUF_SIZE);
    Chip_UART_IntEnable(TRAIN_UART,UART_INTEN_RXRDY);
    NVIC_EnableIRQ(TRAIN_UART_IRQ);

    Chip_UART_Init(LASER_UART);
    Chip_UART_ConfigData(LASER_UART,UART_CONF_8N1);
    Chip_UART_SetBaud(LASER_UART,LASER_UART_BAUD);
    Chip_UART_Enable(LASER_UART);
    RingBuffer_Init(&laser_rx_ring,laser_rx_buf,1,BUF_SIZE);
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
void TrainTxBusIdle(void)
{
    Chip_SWM_Init();
    Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, pin_tx_idle); // Train TX
    Chip_SWM_Deinit();
    Chip_UART_TXDisable(TRAIN_UART);
}

void TrainTxBusActive(void)
{
    Chip_SWM_Init();
    Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, pin_tx); // Train TX
    Chip_SWM_Deinit();
    Chip_UART_TXEnable(TRAIN_UART);
}

void TrainWrite(uint8_t* buf)
{
    TrainTxBusActive();
    Delay(2);
    Chip_UART_SendBlocking(TRAIN_UART,&buf[0],strlen((char*)buf));
}

uint8_t TrainRead(uint8_t* buf, uint8_t len)
{
    return Chip_UART_ReadRB(TRAIN_UART,&train_rx_ring,&buf[0],len);
}

uint8_t TrainDataAvailable(void)
{
    return (uint8_t)RingBuffer_GetCount(&train_rx_ring);
}

uint8_t LaserRead(uint8_t* buf, uint8_t len)
{
    return Chip_UART_ReadRB(LASER_UART,&laser_rx_ring,&buf[0],len);
}

uint8_t LaserDataAvailable(void)
{
    return (uint8_t)RingBuffer_GetCount(&laser_rx_ring);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void LedManageRed(void)
{
    static uint8_t old = 0;
    uint8_t new = (LED_JUICY_ALPHA * _led_red) + old - (LED_JUICY_ALPHA * old);
    if (new != old)
    {
        LedSetRed(new);
        ScheduleSingleEvent(LedManageRed,50);
    }
    old = new;
}

void LedManageBlue(void)
{
    static uint8_t old = 0;
    uint8_t new = (LED_JUICY_ALPHA * _led_blue) + old - (LED_JUICY_ALPHA * old);
    if (new != old)
    {
        LedSetBlue(new);
        ScheduleSingleEvent(LedManageBlue,50);
    }
    old = new;
}

void LedSetRed(uint8_t brightness)
{
    Chip_SWM_Init();
    if (brightness)
    {
        Chip_SWM_MovablePinAssign(SWM_SCT_OUT0_O, pin_red_led);
        uint32_t ticks = Chip_SCTPWM_GetTicksPerCycle(LPC_SCT);
        uint32_t val = (ticks * brightness) / 100;
        Chip_SCTPWM_SetDutyCycle(LPC_SCT, PWM_INDEX_RED, val);
    }
    else
    {
        Chip_SWM_MovablePinAssign(SWM_SCT_OUT0_O,pin_red_idle);
        IO_OUTPUT(pin_red_led);
        IO_LOW(pin_red_led);
        Chip_SWM_Deinit();
    }
    Chip_SWM_Deinit();
}

void LedSetBlue(uint8_t brightness)
{
    Chip_SWM_Init();
    if (brightness)
    {
        Chip_SWM_MovablePinAssign(SWM_SCT_OUT1_O, pin_blue_led);
        uint32_t ticks = Chip_SCTPWM_GetTicksPerCycle(LPC_SCT);
        uint32_t val = (ticks * brightness) / 100;
        Chip_SCTPWM_SetDutyCycle(LPC_SCT, PWM_INDEX_BLUE, val);
    }
    else
    {
        Chip_SWM_MovablePinAssign(SWM_SCT_OUT1_O,pin_blue_idle);
        IO_OUTPUT(pin_blue_led);
        IO_LOW(pin_blue_led);
    }
    Chip_SWM_Deinit();
}

void LedReadRed(uint8_t data[2])
{
    uint8_t brightness[2] = {0};
    brightness[0] = (_led_red / 10) + '0';
    brightness[1] = (_led_red % 10) + '0';
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0],DEVICE_RED_LED,(uint8_t*)&brightness);
    TrainWrite(&buf[0]);
}

void LedWriteRed(uint8_t data[2])
{
    _led_red = ((data[0] - '0') * 10) + (data[1] - '0');
    LedManageRed();
    uint8_t buf[BUF_SIZE] = {0};
    SciWriteResponseCreate(&buf[0],DEVICE_RED_LED);
    TrainWrite(&buf[0]);
}

void LedReadBlue(uint8_t data[2])
{
    uint8_t brightness[2] = {0};
    brightness[0] = (_led_blue / 10) + '0';
    brightness[1] = (_led_blue % 10) + '0';
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0],DEVICE_BLUE_LED,(uint8_t*)&brightness);
    TrainWrite(&buf[0]);
}

void LedWriteBlue(uint8_t data[2])
{
    _led_blue = ((data[0] - '0') * 10) + (data[1] - '0');
    LedManageBlue();
    uint8_t buf[BUF_SIZE] = {0};
    SciWriteResponseCreate(&buf[0],DEVICE_BLUE_LED);
    TrainWrite(&buf[0]);
}

void HitIdRead(uint8_t data[2])
{
    uint8_t id[2] = {0};
    id[0] = (_hit_id / 10) + (_the_bomb == ALL_YOUR_BASE ? '0' : _the_bomb == ARE_BELONG ? YOU_ARE_ON_THE_WAY : TO_DESTRUCTION);
    id[1] = (_hit_id % 10) + '0';
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0], DEVICE_HIT_ID, (uint8_t*)&id);
    TrainWrite(&buf[0]);
}

void HitIdWrite(uint8_t data[2])
{
    _hit_id = ((data[0] - '0') * 10) + (data[1] - '0');
    _the_bomb = ALL_YOUR_BASE;
    uint8_t buf[BUF_SIZE] = {0};
    SciWriteResponseCreate(&buf[0], DEVICE_HIT_ID);
    TrainWrite(&buf[0]);
}

void HitIdWriteSilent(uint8_t data[2])
{
    _hit_id = ((data[0] - '0') * 10) + (data[1] - '0');
}

#endif
