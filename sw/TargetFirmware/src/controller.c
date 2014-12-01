#include "global.h"
#include "controller.h"
#include "controller_hw.h"
#include "sci.h"
#include "train.h"
#include "schedule.h"
#include "delay.h"
#include <string.h>

#if defined (CONTROLLER)
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Local
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void HardwareInit(void);
void UartInit(void);
void RfWrite(uint8_t* buf);
uint8_t RfRead(uint8_t* buf, uint8_t len);
uint8_t RfDataAvailable(void);
void TargetWrite(uint8_t* buf);
uint8_t TargetRead(uint8_t* buf, uint8_t len);
void TargetHandoff(uint8_t* buf);
void TrainVoltageRead(uint8_t data[2]);
void TrainSpeedRead(uint8_t data[2]);
void TrainSpeedWrite(uint8_t data[2]);

#define RING_BUF_SIZE 32
#define BUF_SIZE      10

#define RF_UART             LPC_USART0
#define RF_UART_IRQ         UART0_IRQn
#define RF_UART_HANDLER     UART0_IRQHandler
STATIC  RINGBUFF_T          rf_rx_ring;
static  uint8_t             rf_rx_buf[RING_BUF_SIZE];

#define TARGET_UART         LPC_USART1
#define TARGET_UART_IRQ     UART1_IRQn
#define TARGET_UART_HANDLER UART1_IRQHandler
STATIC  RINGBUFF_T          target_rx_ring;
static  uint8_t             target_rx_buf[RING_BUF_SIZE];

const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

typedef struct
{
    uint8_t type;
    uint8_t device;
    void (*HandlerFn)(uint8_t*);
} CommandRoute;

CommandRoute routes[] =
{
    {COMMAND_TYPE_READ,  DEVICE_VOLTAGE, TrainVoltageRead},
    {COMMAND_TYPE_READ,  DEVICE_SPEED,   TrainSpeedRead},
    {COMMAND_TYPE_WRITE, DEVICE_SPEED,   TrainSpeedWrite}
};

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
        // wait for data
        if (RfDataAvailable())
        {
            // Wait a little for the whole packet to arrive
            Delay(3);
            // Read it from the buffer
            uint8_t buf[BUF_SIZE] = {0};
            RfRead(&buf[0],BUF_SIZE);
            // Pull the data out
            SciCommand command = SciParse(&buf[0]);
            // Not valid?
            if (command.type == COMMAND_TYPE_INVALID)
            {
                // Send an error
                SciErrorResponseCreate(&buf[0],command.error);
                RfWrite(&buf[0]);
                continue;
            }
            // For us?
            if (command.address != MY_ADDRESS)
            {
                uint8_t i = 0;
                for (i = 0;i < (sizeof(routes)/sizeof(CommandRoute));i++)
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
void HardwareInit(void)
{

}

void UartInit(void)
{
    // Enable switch matrix clock
    Chip_SWM_Init();
    // Disable fixed functions for the tx/rx pins
    Chip_SWM_DisableFixedPin(SWM_FIXED_ADC9);  // TX
    Chip_SWM_DisableFixedPin(SWM_FIXED_ADC10); // RX
    // TODO: I2c open drain works for uart?
    Chip_SWM_DisableFixedPin(SWM_FIXED_I2C0_SCL); // RF_TX
    Chip_SWM_DisableFixedPin(SWM_FIXED_I2C0_SDA); // RF_RX

    // Uart div clock by 1
    Chip_Clock_SetUARTClockDiv(1);

    // Assign pin locations for tx/rx/rf_tx/rf_rx
    Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, pin_rf_tx); // RF_TX
    Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, pin_rf_rx); // RF_RX
    Chip_SWM_MovablePinAssign(SWM_U1_TXD_O, pin_tx); // TX
    Chip_SWM_MovablePinAssign(SWM_U1_RXD_I, pin_rx); // RX

    // Disable switch matrix clock (saves power)
    Chip_SWM_Deinit();

    // Init the Uart hardware
    Chip_UART_Init(RF_UART);
    Chip_UART_ConfigData(RF_UART,UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
    Chip_UART_SetBaud(RF_UART,115200);
    Chip_UART_Enable(RF_UART);
    Chip_UART_TXEnable(RF_UART);

    Chip_UART_Init(TARGET_UART);
    Chip_UART_ConfigData(TARGET_UART,UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
    Chip_UART_SetBaud(TARGET_UART,115200);
    Chip_UART_Enable(TARGET_UART);
    Chip_UART_TXEnable(TARGET_UART);

    // Init ring buffers
    RingBuffer_Init(&rf_rx_ring,rf_rx_buf,1,RING_BUF_SIZE);
    RingBuffer_Init(&target_rx_ring,target_rx_buf,1,RING_BUF_SIZE);

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
    Chip_UART_SendBlocking(RF_UART,&buf,strlen((char*)buf));
}

uint8_t RfRead(uint8_t* buf, uint8_t len)
{
    return Chip_UART_ReadRB(RF_UART,&rf_rx_ring,&buf,len);
}

uint8_t RfDataAvailable(void)
{
    return (uint8_t)RingBuffer_GetCount(&rf_rx_ring);
}

void TargetWrite(uint8_t* buf)
{
    Chip_UART_SendBlocking(TARGET_UART,&buf,strlen((char*)buf));
}

uint8_t TargetRead(uint8_t* buf, uint8_t len)
{
    return Chip_UART_ReadRB(TARGET_UART,&target_rx_ring,&buf,len);
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
void TrainVoltageRead(uint8_t data[2])
{
    //TODO: ADC stuff
    uint8_t reading[2] = {0};
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0],reading);
    RfWrite(&buf[0]);
}

void TrainSpeedRead(uint8_t data[2])
{
    TrainSpeed ts = TrainGetSpeed();
    uint8_t buf[BUF_SIZE] = {0};
    SciReadResponseCreate(&buf[0],(uint8_t*)ts.speed);
    RfWrite(&buf[0]);
}

void TrainSpeedWrite(uint8_t data[2])
{
    TrainSpeed ts = {{0}};
    memcpy(&ts,&data,2);
    TrainSetSpeed(ts);
    uint8_t buf[BUF_SIZE] = {0};
    SciWriteResponseCreate(&buf[0],MY_ADDRESS);
    RfWrite(&buf[0]);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


#endif
