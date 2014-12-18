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
static void    RfService(void);
static void    AdcInit(void);
static void    PwmInit(void);
static void    PwmEnable(void);
static void    PwmDisable(void);
static void    UartInit(void);
static void    RfWrite(uint8_t* buf);
static uint8_t RfRead(uint8_t* buf, uint8_t len);
static uint8_t RfDataAvailable(void);
static void    TargetWrite(uint8_t* buf);
static uint8_t TargetRead(uint8_t* buf, uint8_t len);
static void    TargetHandoff(uint8_t* buf);
static void    VoltageRead(uint8_t data[2]);
static void    SpeedRead(uint8_t data[2]);
static void    SpeedWrite(uint8_t data[2]);

// Uart stuff
#define BUF_SIZE            128

#define RF_UART             LPC_USART0
#define RF_UART_IRQ         UART0_IRQn
#define RF_UART_HANDLER     UART0_IRQHandler
STATIC  RINGBUFF_T          rf_rx_ring;
static  uint8_t             rf_rx_buf[BUF_SIZE];

#define TARGET_UART         LPC_USART1
#define TARGET_UART_IRQ     UART1_IRQn
#define TARGET_UART_HANDLER UART1_IRQHandler
STATIC  RINGBUFF_T          target_rx_ring;
static  uint8_t             target_rx_buf[BUF_SIZE];

#define UART_CONF_8N1       (UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1)
#define UART_BAUD_RATE      115200

// Clock config
const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

// Commad routers
typedef struct
{
    uint8_t type;
    uint8_t device;
    void (*HandlerFn)(uint8_t*);
} CommandRoute;

// Pwm settings
#define PWM_RATE            40000
#define PWN_INDEX           1
#define SPEED_SIG_REPEAT    10

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Entry
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
inline void Run(void)
{
    ScheduleInit();
    PwmInit();
    UartInit();
    AdcInit();
    for(;;)
    {
        RfService();
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

    if (RfDataAvailable())
    {
        // Wait a little for the whole packet to arrive
        Delay(3);
        uint8_t buf[BUF_SIZE] = {0};
        RfRead(&buf[0],BUF_SIZE);
        SciCommand command = SciParse(&buf[0]);
        if (command.type == COMMAND_TYPE_INVALID)
        {
            memset(buf,0,sizeof(buf));
            SciErrorResponseCreate(&buf[0],command.error);
            RfWrite(&buf[0]);
        }
        else
        {
            if (command.address == MY_ADDRESS)
            {
                for (int i = 0;i < (sizeof(routes)/sizeof(CommandRoute));i++)
                {
                    if (command.type   == routes[i].type &&
                        command.device == routes[i].device)
                    {
                        routes[i].HandlerFn(command.data);
                        break;
                    }
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

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Init
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void PwmInit(void)
{
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
    Chip_SWM_MovablePinAssign(SWM_SCT_OUT0_O, pin_ir);
    Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);

    Chip_SCTPWM_Init(LPC_SCT);
    Chip_SCTPWM_SetRate(LPC_SCT, PWM_RATE);
    Chip_SCTPWM_SetOutPin(LPC_SCT, PWN_INDEX, 0);
    Chip_SCTPWM_SetDutyCycle(LPC_SCT, PWN_INDEX, Chip_SCTPWM_GetTicksPerCycle(LPC_SCT)/2);
}

void PwmEnable(void)
{
    Chip_SCTPWM_Start(LPC_SCT);
}

void PwmDisable(void)
{
    Chip_SCTPWM_Stop(LPC_SCT);
}

void AdcInit(void)
{
    Chip_ADC_Init(LPC_ADC, 0);
    Chip_ADC_StartCalibration(LPC_ADC);
    while (!(Chip_ADC_IsCalibrationDone(LPC_ADC)));
    Chip_ADC_SetClockRate(LPC_ADC, ADC_MAX_SAMPLE_RATE);
    Chip_ADC_SetupSequencer(LPC_ADC, ADC_SEQA_IDX, ADC_SEQ_CTRL_CHANSEL(3));
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
    Chip_SWM_EnableFixedPin(SWM_FIXED_ADC3);
    Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);

    Chip_ADC_ClearFlags(LPC_ADC, Chip_ADC_GetFlags(LPC_ADC));
    Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQA_IDX);
}

void UartInit(void)
{
    // Enable switch matrix clock
    Chip_SWM_Init();
    // Disable fixed functions for the tx/rx pins
    Chip_SWM_DisableFixedPin(SWM_FIXED_ADC9);  // TX
    Chip_SWM_DisableFixedPin(SWM_FIXED_ADC10); // RX
    Chip_SWM_DisableFixedPin(SWM_FIXED_I2C0_SCL); // RF_TX
    Chip_SWM_DisableFixedPin(SWM_FIXED_I2C0_SDA); // RF_RX

    // Uart div clock by 1
    Chip_Clock_SetUARTClockDiv(1);
    Chip_Clock_SetUSARTNBaseClockRate((UART_BAUD_RATE * 16), true);

    // Assign pin locations for tx/rx/rf_tx/rf_rx
    Chip_SWM_MovablePinAssign(SWM_U1_TXD_O, pin_tx); // TX
    Chip_SWM_MovablePinAssign(SWM_U1_RXD_I, pin_rx); // RX
    Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, pin_rf_tx); // RF_TX
    Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, pin_rf_rx); // RF_RX

    // Disable switch matrix clock (saves power)
    Chip_SWM_Deinit();

    // Init the Uart hardware
    Chip_UART_Init(RF_UART);
    Chip_UART_ConfigData(RF_UART,UART_CONF_8N1);
    Chip_UART_SetBaud(RF_UART,UART_BAUD_RATE);
    Chip_UART_Enable(RF_UART);
    Chip_UART_TXEnable(RF_UART);

    Chip_UART_Init(TARGET_UART);
    Chip_UART_ConfigData(TARGET_UART,UART_CONF_8N1);
    Chip_UART_SetBaud(TARGET_UART,UART_BAUD_RATE);
    Chip_UART_Enable(TARGET_UART);
    Chip_UART_TXEnable(TARGET_UART);

    // Init ring buffers
    RingBuffer_Init(&rf_rx_ring,rf_rx_buf,1,BUF_SIZE);
    RingBuffer_Init(&target_rx_ring,target_rx_buf,1,BUF_SIZE);

    // Enable interrupts
    Chip_UART_IntEnable(RF_UART,UART_INTEN_RXRDY);
    NVIC_EnableIRQ(RF_UART_IRQ);
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

void TargetHandoff(uint8_t* buf)
{
    TargetWrite(&buf[0]);
    Delay(10);
    memset(buf,0,BUF_SIZE);
    TargetRead(&buf[0],BUF_SIZE);
    RfWrite(&buf[0]);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Command Handlers
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void VoltageRead(uint8_t data[2])
{
    Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
    //DelayDumb(1000);
    uint32_t sample = 0;
    do
    {
        sample = Chip_ADC_GetDataReg(LPC_ADC,3);
    } while (!(sample & ADC_SEQ_GDAT_DATAVALID));
    sample = ADC_DR_RESULT(sample);
    // 12bit 0-3V DC convert to 100mV scale, with V/2 divider
    sample = (sample * 60) / 0xFFF;
    uint8_t reading[2] = {0};
    reading[0] = (sample / 10) + '0';
    reading[1] = (sample % 10) + '0';
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0],reading);
    RfWrite(&buf[0]);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
static uint8_t _speed[2] = {'S','T'};

void SpeedRead(uint8_t data[2])
{
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0],&_speed[0]);
    RfWrite(&buf[0]);
}

void SpeedWrite(uint8_t data[2])
{
    uint8_t buf[BUF_SIZE] = {0};
    memcpy(&_speed[0],&data[0],2);

    const uint16_t speed_r1[] = {275,281,283,284,287,288,291,292,293};
    const uint16_t speed_st[] = {275,281,283,286,287,288,291,294,297};
    const uint16_t speed_f1[] = {275,281,283,286,287,290,291,292,293};
    const uint16_t speed_f2[] = {275,281,283,284,287,290,291,294,297};
    const uint16_t* speed_sel = (memcmp(_speed,SPEED_SLOW_ASTERN,2) == 0) ? speed_r1 :
                                (memcmp(_speed,SPEED_HALF_AHEAD,2)  == 0) ? speed_f1 :
                                (memcmp(_speed,SPEED_FULL_AHEAD,2)  == 0) ? speed_f2 : speed_st;

    for (uint8_t i = 0;i < 9;i++)
    {
        ScheduleSingleEvent((i % 2 == 0) ? PwmDisable : PwmEnable, speed_sel[i]);
    }
    SciWriteResponseCreate(&buf[0],MY_ADDRESS);
    RfWrite(&buf[0]);
}

#endif
