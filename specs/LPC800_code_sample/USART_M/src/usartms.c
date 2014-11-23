/****************************************************************************
 *   $Id:: uarttest.c 3635 2012-10-31 00:31:46Z usb00423                    $
 *   Project: NXP LPC8xx USART example
 *
 *   Description:
 *     This file contains USART test modules, main entry, to test USART APIs.
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
#include "LPC8xx.h"
#include "lpc8xx_uart.h"
#include "lpc8xx_gpio.h"

extern volatile uint8_t UARTTxBuffer[BUFSIZE];
extern volatile uint8_t UARTRxBuffer[BUFSIZE];
extern volatile uint32_t UARTBlockReceived;

/*****************************************************************************
** Function name:		Main
**
** Descriptions:		Main entry
** parameters:			None
** Returned value:		int
** 
*****************************************************************************/
int main (void) {                       /* Main Program */
  uint32_t i;
  uint32_t regVal;

  SystemCoreClockUpdate();

	GPIOInit();
	GPIOSetDir(0, 7, 0);

	/*
	 * Initialize UART0 pin connect
	 */
	/*connect the UART TXD abd RXD sigals to port pins(P0.4, P0.0), SCLK to P0.6 */
	regVal = LPC_SWM->PINASSIGN0;
	regVal &= ~( 0xFF << 0 );
	regVal |= ( 4 << 0 );
	LPC_SWM->PINASSIGN0 = regVal;	/* P0.4 is UART TXD, ASSIGN0(7:0). */
	regVal = LPC_SWM->PINASSIGN0;
	regVal &= ~( 0xFF << 8 );
	regVal |= ( 0 << 8 );
	LPC_SWM->PINASSIGN0 = regVal;	/* P0.0 is UART RXD, ASSIGN0(15:8). */
	regVal = LPC_SWM->PINASSIGN1;
	regVal &= ~( 0xFF << 0 );
	regVal |= ( 6 << 0 );
	LPC_SWM->PINASSIGN1 = regVal;	/* P0.6 is UART SCLK, ASSIGN0(7:0). */

  for ( i = 0; i < BUFSIZE; i++ )
  {
		UARTTxBuffer[i] = i + 1 + 0x55;
		if ( (i & 7) == 7 )
		{
			UARTTxBuffer[i] = 0;
		}	
		UARTRxBuffer[i] = 0;
  }

  while ( GPIOGetPinValue(0, 7) == 0x1);
	
  UARTBlockReceived = 0;

  USARTInit(LPC_USART0, 115200, MASTER_MODE);		/* master mode */
#if TX_DISABLE
  LPC_USART0->INTENSET = TXINT_DIS;
#endif

#if !HALF_DUPLEX
  UARTSend( LPC_USART0, (uint8_t *)UARTTxBuffer, BUFSIZE );
#endif

#if TX_DISABLE
  LPC_USART0->CTRL = TXINT_DIS;
#endif

  while ( !UARTBlockReceived );		/* Whole data block received from the slave. */

  for ( i = 0; i < BUFSIZE; i++ )
  {
		if ( (i & 7) == 7 )
		{
			if ( UARTRxBuffer[i] != 0 )
			{
				while ( 1 );
			}
		}
		else
		{
			if ( UARTRxBuffer[i] != (i + 1 + 0xAA) )
			{
				while ( 1 );
			}
		}	
  }

  while ( 1 );	/* never exit from main for debugging. */
}
