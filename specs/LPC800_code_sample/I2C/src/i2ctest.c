/****************************************************************************
 *   $Id:: i2ctest.c 3662 2012-10-31 19:47:02Z usb00423                     $
 *   Project: NXP LPC8xx I2C example
 *
 *   Description:
 *     This file contains I2C test modules, main entry, to test I2C APIs.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation is hereby granted, under NXP Semiconductors' 
 * relevant copyright in the software, without fee, provided that it 
 * is used in conjunction with NXP Semiconductors microcontrollers. This 
 * copyright, permission, and disclaimer notice must appear in all copies of 
 * this code.
****************************************************************************/
#include "LPC8xx.h"			/* LPC8xx Peripheral Registers */
#include "type.h"
#include "lpc8xx_i2c.h"

volatile uint8_t I2CMasterTXBuffer[I2C_BUFSIZE];
volatile uint8_t I2CMasterRXBuffer[I2C_BUFSIZE];
volatile uint8_t I2CSlaveTXBuffer[I2C_BUFSIZE];
volatile uint8_t I2CSlaveRXBuffer[I2C_BUFSIZE];
volatile uint32_t I2CMonBuffer[I2C_MONBUFSIZE];
volatile uint32_t I2CReadLength, I2CWriteLength;

/*****************************************************************************
** Function name:		Main
**

** Descriptions:		Main entry
** parameters:			None
** Returned value:		int
** 
*****************************************************************************/
int main (void) 
{                       /* Main Program */
  uint32_t i, regVal;
  
  SystemCoreClockUpdate();

	/*
	 * Initialize I2C pin connect
	 */
#if 1
	/*connect the I2C SCL and SDA sigals to port pins(P0.10-P0.11)*/
  regVal = LPC_SWM->PINASSIGN7 & ~(0xFFUL<<24);
  LPC_SWM->PINASSIGN7 = regVal | (10 << 24);	/* P0.10 is I2C SDA, ASSIGN0(31:24) */
  regVal = LPC_SWM->PINASSIGN8 & ~(0xFF<<0);
  LPC_SWM->PINASSIGN8 = regVal | (11 << 0);	/* P0.11 is I2C SCL. ASSIGN0(7:0) */
	regVal = LPC_IOCON->PIO0_10 & ~(0x3<<8);			
	LPC_IOCON->PIO0_10 = regVal | (0x2<<8);							/* Enable Fast Mode Plus */
	regVal = LPC_IOCON->PIO0_11 & ~(0x3<<8);			
	LPC_IOCON->PIO0_11 = regVal | (0x2<<8);							/* Enable Fast Mode Plus */
#else
	/* Test pseudo open  drain, connect the I2C SCL and SDA sigals to port pins(P0.7-P0.8)*/
  regVal = LPC_SWM->PINASSIGN7 & ~(0xFFUL<<24);
  LPC_SWM->PINASSIGN7 = regVal | (14 << 24);		/* P0.14 is I2C SDA, ASSIGN0(31:24) */
  regVal = LPC_SWM->PINASSIGN8 & ~(0xFF<<0);
  LPC_SWM->PINASSIGN8 = regVal | (15 << 0);		/* P0.15 is I2C SCL. ASSIGN0(7:0) */			
	LPC_IOCON->PIO0_14 |= (0x1<<10);							/* Enable pseudo open drain */		
	LPC_IOCON->PIO0_15 |= (0x1<<10);							/* Enable pseudo open drain */
#endif
	
	/* Enable I2C clock */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<5);
	/* Toggle peripheral reset control to I2C, a "1" bring it out of reset. */
	LPC_SYSCON->PRESETCTRL &= ~(0x1<<6);
	LPC_SYSCON->PRESETCTRL |= (0x1<<6);

#if I2C_MONITOR_MODE
  /* Monitor RX data is only handled inside interrupt handler. */
  I2C_MonInit( LPC_I2C, CFG_MONENA );
  for ( i = 0; i < I2C_MONBUFSIZE; i++ )	/* clear monitor buffer */
  {
    I2CMonBuffer[i] = 0;
  }
#endif
#if TIMEOUT_ENABLED
  I2C_TimeoutInit( LPC_I2C, TIMEOUT_VALUE );
#endif

  I2C_SlvInit(LPC_I2C, SLAVE_ADDR, CFG_SLVENA, I2C_FMODE_PRE_DIV);
  /* Ready to receive the slave address, interrupt(SLAVE ADDR STATE) if match occurs. */
  LPC_I2C->SLVCTL = CTL_SLVCONTINUE;
#if I2C_INTERRUPT
  LPC_I2C->INTENSET = STAT_SLVPEND;  
#endif
  
  /* For master mode plus, if desired I2C clock is 1MHz (SCL high time + SCL low time). 
	If CCLK is 36MHz, MasterSclLow and MasterSclHigh are 0s, 
	SCL high time = (ClkDiv+1) * (MstSclHigh + 2 )
	SCL low time = (ClkDiv+1) * (MstSclLow + 2 )
	Pre-divider should be 36000000/(1000000*4)-1 = 9-1 = 8. 
	If fast mode, e.g. communicating with a temp sensor, Max I2C clock is set to 400KHz.
	Pre-divider should be 36000000/(400000*4)-1 = 22.5-1 = 22.
	If standard mode, e.g. communicating with a temp sensor, Max I2C clock is set to 100KHz.
	Pre-divider should be 36000000/(100000*4)-1 = 90-1 = 89. */
  I2C_MstInit(LPC_I2C, I2C_FMODE_PRE_DIV, CFG_MSTENA, 0x00);
  						
  /* Get device ID register */
  for ( i = 0; i < I2C_BUFSIZE; i++ )	/* clear buffer */
  {
		I2CMasterTXBuffer[i] = 0;
		I2CMasterRXBuffer[i] = 0;
		I2CSlaveTXBuffer[i] = 0;
		I2CSlaveRXBuffer[i] = 0;
  }

  I2C_CheckIdle(LPC_I2C);
	
  /* If I2C_INTERRUPT is enabled, SLAVE TX and RX are handled inside the ISR. The
  master can send a packet (WRITE or READ) with I2C_BUFSIZE to this device, 
  I2C_SlaveReceiveSend(0 will be handled inside ISR. */
#if !I2C_INTERRUPT
#if SLAVE_POLLING_TEST
  /* If SLAVE POLLING_TEST is enabled, Master mode(temperature sensor reading) cannot 
  be tested at the same time. */
  /* Slave receive and transmit test. */
  while ( 1 )
  {
		I2C_SlaveReceiveSend( LPC_I2C, (uint8_t *)I2CSlaveTXBuffer, (uint8_t *)I2CSlaveRXBuffer, I2C_BUFSIZE );
		for ( i = 0; i < I2C_BUFSIZE; i++ )	/* clear buffer */
		{
			/* copy data from SlaveReceive buffer and send the received data back 
			to the master if SlaveSend() is seen. */
			I2CSlaveTXBuffer[i] = I2CSlaveRXBuffer[i];
		}
  }
#endif
#endif

  /* I2C also works as an I2C master communicate with the temperature sensor(SE95). 
  The sequence is, 
  (1) GET TEMP DEVICE ID (0x90(W), 0x05(cmd), 0x91(R), 0xA1(Dev ID).
  (2) Set Configuration (to default), 0x90, 0x01(cmd), 0x00.
  (3) Get temperature reading 0x90(W), 0x00, 0x91(R), 0xAB, 0xCD. where 0xABCD is 
  the temp reading.
  */
  while ( 1 )					/* Enable while(1) to run stress test and multi-master ARB loss test. */
  {
#if 1
		/* Test master send and receive with repeated start */
    I2CMasterTXBuffer[0] = SE95_ID;
		I2C_MstSendRcv( LPC_I2C, SE95_ADDR, (uint8_t *)I2CMasterTXBuffer, 1, (uint8_t *)I2CMasterRXBuffer, 1 );
#else
		/* Test master send and receive separately. */
		I2CMasterTXBuffer[0] = SE95_ID;
		I2C_MstSend( LPC_I2C, SE95_ADDR, (uint8_t *)I2CMasterTXBuffer, 1 );
		I2C_MstReceive( LPC_I2C, SE95_ADDR|RD_BIT, (uint8_t *)I2CMasterRXBuffer, 1 );  
#endif
  }
  
  /* Configure temp register before reading */
  for ( i = 0; i < I2C_BUFSIZE; i++ )	/* clear buffer */
  {
		I2CMasterTXBuffer[i] = 0;
  }

  I2CMasterTXBuffer[0] = SE95_CONFIG;
  I2CMasterTXBuffer[1] = 0x00;		/* configuration value, no change from default */
  I2C_MstSend( LPC_I2C, SE95_ADDR, (uint8_t *)I2CMasterTXBuffer, 2 );
	
  /* Get temp reading */
  for ( i = 0; i < I2C_BUFSIZE; i++ )	/* clear buffer */
  {
		I2CMasterTXBuffer[i] = 0;
		I2CMasterRXBuffer[i] = 0;
  }
  I2CMasterTXBuffer[0] = SE95_TEMP;
  I2C_MstSendRcv( LPC_I2C, SE95_ADDR, (uint8_t *)I2CMasterTXBuffer, 1, (uint8_t *)I2CMasterRXBuffer, 2 );
  /* The temp reading value should reside in I2CMasterRXBuffer... */
  
  while ( 1 );

}

/******************************************************************************
**                            End Of File
******************************************************************************/
