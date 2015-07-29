#include "global.h"
#include "controller.h"
#include "controller_hw.h"
#include "sci.h"
#include "schedule.h"
#include "delay.h"
#include <string.h>

#if defined (CONTROLLER)
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Local
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Uart stuff
#define BUF_SIZE            16

#define RF_UART             LPC_USART1
#define RF_UART_IRQ         UART1_IRQn
#define RF_UART_HANDLER     UART1_IRQHandler
#define RF_TX_PORT          SWM_U1_TXD_O
#define RF_RX_PORT          SWM_U1_RXD_I
STATIC  RINGBUFF_T          rf_rx_ring;
static  uint8_t             rf_rx_buf[BUF_SIZE];

#define TARGET_UART         LPC_USART0
#define TARGET_UART_IRQ     UART0_IRQn
#define TARGET_UART_HANDLER UART0_IRQHandler
#define TARGET_TX_PORT      SWM_U0_TXD_O
#define TARGET_RX_PORT      SWM_U0_RXD_I
STATIC  RINGBUFF_T          target_rx_ring;
static  uint8_t             target_rx_buf[BUF_SIZE];

#define UART_CONF_8N1       (UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1)
#define UART_BAUD_RATE      57600

// Clock config
const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

typedef void (*HandlerFn)(uint8_t*);
typedef struct
{
    uint8_t type;
    uint8_t device;
    HandlerFn handler;
} CommandRoute;

// Pwm settings (train IR)
#define PWM_RATE            40000
#define PWN_INDEX           1

static uint16_t speed_r1[] = {0,6,2,1,3,1,3,1};
static uint16_t speed_st[] = {0,6,2,3,1,1,3,3};
static uint16_t speed_f1[] = {0,6,2,3,1,3,1,1};
static uint16_t speed_f2[] = {0,6,2,1,3,3,1,3};
static uint16_t *cur_speed_pattern = speed_st;

static void      RfService(void);
static void      TargetService(void);
static HandlerFn HandlerSearch(SciCommand command, CommandRoute* routes, uint8_t len);
static void      AdcInit(void);
static void      AdcEnable(void);
static void      AdcDisable(void);
static void      LedIrInit(void);
static void      LedIrEnable(void);
static void      LedIrDisable(void);
static void      LedIrFunction(void);
static void      UartInit(void);
static void      RfWrite(uint8_t* buf);
static uint8_t   RfRead(uint8_t* buf, uint8_t len);
static uint8_t   RfDataAvailable(void);
static void      TargetWrite(uint8_t* buf);
static uint8_t   TargetRead(uint8_t* buf, uint8_t len);
static uint8_t   TargetDataAvailable(void);
static void      TargetHandoff(uint8_t* buf);
static void      VoltageRead(uint8_t data[2]);
static void      SpeedRead(uint8_t data[2]);
static void      SpeedWrite(uint8_t data[2]);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Entry
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
inline void Run(void)
{
    ScheduleInit();
    LedIrInit();
    UartInit();
    AdcInit();

#ifdef CONTROLLER_TEST_SERIAL
    RfWrite("RF link up\n");
    TargetWrite("Target link up\n");
    while(1)
    {
        if (TargetDataAvailable())
        {
            uint8_t buf[BUF_SIZE] = {0};
            TargetRead(&buf[0],BUF_SIZE);
            RfWrite(&buf[0]);
        }
        if (RfDataAvailable())
        {
            uint8_t buf[BUF_SIZE] = {0};
            RfRead(&buf[0],BUF_SIZE);
            TargetWrite(&buf[0]);
        }
    }
#endif

    LedIrFunction();

    for(;;)
    {
        RfService();
        TargetService();
    }
}


void RfService(void)
{
    CommandRoute routes[] =
    {
        {COMMAND_TYPE_READ,  DEVICE_VOLTAGE, VoltageRead},
        {COMMAND_TYPE_READ,  DEVICE_SPEED,   SpeedRead},
        {COMMAND_TYPE_WRITE, DEVICE_SPEED,   SpeedWrite}
    };
    uint8_t route_len = sizeof(routes) / sizeof(CommandRoute);

    if (RfDataAvailable())
    {
        // Wait a little for the whole packet to arrive
        Delay(3);
        uint8_t buf[BUF_SIZE] = {0};
        RfRead(&buf[0],BUF_SIZE);
        SciCommand command = SciParse(&buf[0]);
        if (command.type == COMMAND_TYPE_INVALID)
        {
            //uint8_t err_buf[6] = "?:";
            //SciErrorResponseCreate(&err_buf[2],command.error);
            //uint8_t len = strlen(buf);
            //strcpy(&buf[len-1],err_buf);
            memset(buf,0,sizeof(buf));
            SciErrorResponseCreate(&buf[0],command.error);
            RfWrite(&buf[0]);
        }
        else
        {
            if (command.address == MY_ADDRESS)
            {
                HandlerFn fn = HandlerSearch(command,&routes[0],route_len);
                if (fn != NULL)
                {
                    fn(command.data);
                }
            }
            // for the targets
            else
            {
                TargetHandoff(&buf[0]);
            }
        }
    }
}

void TargetService(void)
{
    if (TargetDataAvailable())
    {
        // Wait a little for the whole packet to arrive
        Delay(5);
        uint8_t buf[BUF_SIZE] = {0};
        TargetRead(&buf[0],BUF_SIZE);
        RfWrite(&buf[0]);
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
void LedIrInit(void)
{
    Chip_SCTPWM_Init(LPC_SCT);
    Chip_SCTPWM_SetRate(LPC_SCT, PWM_RATE);
    Chip_SCTPWM_SetDutyCycle(LPC_SCT, PWN_INDEX, Chip_SCTPWM_GetTicksPerCycle(LPC_SCT)/2);
}

void LedIrEnable(void)
{
    Chip_SWM_Init();
    Chip_SWM_MovablePinAssign(SWM_SCT_OUT0_O, pin_ir);
    Chip_SWM_Deinit();
    Chip_SCTPWM_SetOutPin(LPC_SCT, PWN_INDEX, 0);
    Chip_SCTPWM_Start(LPC_SCT);
}

void LedIrDisable(void)
{
    Chip_SCTPWM_Stop(LPC_SCT);
    Chip_SWM_Init();
    Chip_SWM_MovablePinAssign(SWM_SCT_OUT0_O, pin_ir_idle);
    IO_OUTPUT(pin_ir);
    IO_HIGH(pin_ir);
    Chip_SWM_Deinit();
}

void LedIrFunction(void)
{
    LedIrDisable();
    uint16_t offset = 1;
    for (uint8_t j = 0;j < 8;j++)
    {
        offset += cur_speed_pattern[j];
        ScheduleSingleEvent((j % 2 == 0) ? LedIrEnable : LedIrDisable, offset);
    }
    ScheduleSingleEvent(LedIrFunction, 280);
}

void AdcInit(void)
{
    Chip_ADC_Init(LPC_ADC, 0);
    Chip_ADC_StartCalibration(LPC_ADC);
    while (!(Chip_ADC_IsCalibrationDone(LPC_ADC)));
    Chip_ADC_SetClockRate(LPC_ADC, ADC_MAX_SAMPLE_RATE);
    Chip_ADC_SetupSequencer(LPC_ADC, ADC_SEQA_IDX, ADC_SEQ_CTRL_CHANSEL(3));
    Chip_ADC_ClearFlags(LPC_ADC, Chip_ADC_GetFlags(LPC_ADC));
    Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQA_IDX);
    Chip_SWM_Init();
    Chip_SWM_EnableFixedPin(SWM_FIXED_ADC3);
    Chip_SWM_Deinit();
}

void AdcEnable(void)
{
    Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
}

void AdcDisable(void)
{
    Chip_ADC_DisableSequencer(LPC_ADC,ADC_SEQ_CTRL_CHANSEL(3));
}

void UartInit(void)
{
    Chip_SWM_Init();
    // Disable fixed functions for the tx/rx pins
    Chip_SWM_DisableFixedPin(SWM_FIXED_I2C0_SCL); // RF_TX
    Chip_SWM_DisableFixedPin(SWM_FIXED_I2C0_SDA); // RF_RX
    Chip_SWM_DisableFixedPin(SWM_FIXED_ADC9);  // TX
    Chip_SWM_DisableFixedPin(SWM_FIXED_ADC10); // RX

    Chip_Clock_SetUARTClockDiv(1);
    Chip_Clock_SetUSARTNBaseClockRate((UART_BAUD_RATE * 16), true);

    Chip_SWM_MovablePinAssign(RF_TX_PORT, pin_rf_tx);
    Chip_SWM_MovablePinAssign(RF_RX_PORT, pin_rf_rx);
    Chip_SWM_MovablePinAssign(TARGET_TX_PORT, pin_tx);
    Chip_SWM_MovablePinAssign(TARGET_RX_PORT, pin_rx);

    // Disable switch matrix clock (saves power)
    Chip_SWM_Deinit();

    // Init the Uart hardware
    Chip_UART_Init(RF_UART);
    Chip_UART_ConfigData(RF_UART,UART_CONF_8N1);
    Chip_UART_SetBaud(RF_UART,UART_BAUD_RATE);
    Chip_UART_Enable(RF_UART);
    Chip_UART_TXEnable(RF_UART);
    RingBuffer_Init(&rf_rx_ring,rf_rx_buf,1,BUF_SIZE);
    Chip_UART_IntEnable(RF_UART,UART_INTEN_RXRDY);
    NVIC_EnableIRQ(RF_UART_IRQ);

    Chip_UART_Init(TARGET_UART);
    Chip_UART_ConfigData(TARGET_UART,UART_CONF_8N1);
    Chip_UART_SetBaud(TARGET_UART,UART_BAUD_RATE);
    Chip_UART_Enable(TARGET_UART);
    Chip_UART_TXEnable(TARGET_UART);
    RingBuffer_Init(&target_rx_ring,target_rx_buf,1,BUF_SIZE);
    Chip_UART_IntEnable(TARGET_UART,UART_INTEN_RXRDY);
    NVIC_EnableIRQ(TARGET_UART_IRQ);

}

// Hook into Uart interrupts and redirect them to ring buffers
void RF_UART_HANDLER(void)
{
    Chip_UART_RXIntHandlerRB(RF_UART, &rf_rx_ring);
}

void TARGET_UART_HANDLER(void)
{
    Chip_UART_RXIntHandlerRB(TARGET_UART, &target_rx_ring);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Communication
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void RfWrite(uint8_t* buf)
{
    Chip_UART_SendBlocking(RF_UART,&buf[0],strlen((char*)buf));
}

uint8_t RfRead(uint8_t* buf, uint8_t len)
{
    return Chip_UART_ReadRB(RF_UART,&rf_rx_ring,&buf[0],len);
}

uint8_t RfDataAvailable(void)
{
    return (uint8_t)RingBuffer_GetCount(&rf_rx_ring);
}

void TargetWrite(uint8_t* buf)
{
    Chip_UART_SendBlocking(TARGET_UART,&buf[0],strlen((char*)buf));
}

uint8_t TargetRead(uint8_t* buf, uint8_t len)
{
    return Chip_UART_ReadRB(TARGET_UART,&target_rx_ring,&buf[0],len);
}

uint8_t TargetDataAvailable(void)
{
    return (uint8_t)RingBuffer_GetCount(&target_rx_ring);
}

void TargetHandoff(uint8_t* buf)
{
    TargetWrite(&buf[0]);
    Delay(60);
    memset(buf,0,BUF_SIZE);
    TargetRead(&buf[0],BUF_SIZE);
    RfWrite(&buf[0]);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Command Handlers
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void VoltageRead(uint8_t data[2])
{
    AdcEnable();
    Delay(5);
    uint32_t sample = 0;
    do
    {
        sample = Chip_ADC_GetDataReg(LPC_ADC,3);
    } while (!(sample & ADC_SEQ_GDAT_DATAVALID));
    AdcDisable();
    sample = ADC_DR_RESULT(sample);
    // 12bit 0-3V DC convert to 100mV scale, with V/2 divider
    sample = (sample * 60) / 0xFFF;
    uint8_t reading[2] = {0};
    reading[0] = (sample / 10) + '0';
    reading[1] = (sample % 10) + '0';
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0], DEVICE_VOLTAGE, reading);
    RfWrite(&buf[0]);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
static uint8_t _speed[2] = {'S','T'};

void SpeedRead(uint8_t data[2])
{
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0], DEVICE_SPEED, &_speed[0]);
    RfWrite(&buf[0]);
}

void SpeedWrite(uint8_t data[2])
{
    uint8_t buf[BUF_SIZE] = {0};
    memcpy(&_speed[0],&data[0],2);

    cur_speed_pattern = (memcmp(_speed,SPEED_SLOW_ASTERN,2) == 0) ? speed_r1 :
                        (memcmp(_speed,SPEED_HALF_AHEAD,2)  == 0) ? speed_f1 :
                        (memcmp(_speed,SPEED_FULL_AHEAD,2)  == 0) ? speed_f2 :
                                                                    speed_st;

    SciWriteResponseCreate(&buf[0], DEVICE_SPEED);
    RfWrite(&buf[0]);
}

#endif
