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
#include "lpc8xx_clkconfig.h"
#include "lpc8xx_uart.h"

volatile uint32_t UARTLoopbackCount = 0;

extern volatile uint32_t UARTRxCount;
extern volatile uint32_t RxErrorCount;
extern volatile uint8_t UARTRxBuffer[BUFSIZE];

/*****************************************************************************
** Function name:		LoopbackTest
**
** Descriptions:		For loopback test, the simpliest is to check the
**						RX_DATA and TXDATA register. If flow control is turned 
**						on, make sure it enables internal CTS and disables the 
**						external one. 
**						
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void LoopbackTest( LPC_USART_TypeDef *UARTx )
{
  UARTx->CFG |= LOOPBACK;		/* data loopback */
  UARTx->INTENCLR = 0xFFFF;		/* Disable all interrupts. */

#if FLOWCTRL_ENABLE
  UARTx->CFG &= ~EXT_CTS_EN;    /* Disable external CTS. */
  UARTx->CFG |= INT_CTS_EN;     /* Turn on internal CTS. */
#endif

  while ( 1 )
  {
		UARTx->TXDATA = 'A';
		while ( !(UARTx->STAT & RXRDY));
		if ( UARTx->RXDATA != 'A' )
		{ 
			while ( 1 );	/* Loopback test fails. */
		}
		UARTLoopbackCount++;
  }
}

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

	/* Config CLKOUT, mostly used for debugging. */
	regVal = LPC_SWM->PINASSIGN8;
	regVal &= ~( 0xFF << 16 );
	regVal |= ( 17 << 16 );
	LPC_SWM->PINASSIGN8 = regVal;	/* P0.17 is CLKOUT, ASSIGN(23:16). */
  CLKOUT_Setup( CLKOUTCLK_SRC_MAIN_CLK );
//	CLKOUT_Setup( CLKOUTCLK_SRC_IRC_OSC );

	/*
	 * Initialize UARTx pin connect
	 */
#if 1
  /* connect the UART0 TXD abd RXD sigals to port pins(P0.4-P0.0)*/
	regVal = LPC_SWM->PINASSIGN0 & ~( 0xFF << 0 );
	LPC_SWM->PINASSIGN0 = regVal | ( 4 << 0 );				/* P0.4 is UART0 TXD, ASSIGN0(7:0) */
	regVal = LPC_SWM->PINASSIGN0 & ~( 0xFF << 8 );
	LPC_SWM->PINASSIGN0 = regVal | ( 0 << 8 );				/* P0.0 is UART0 RXD. ASSIGN0(15:8) */
	
	regVal = LPC_SWM->PINASSIGN0 & ~( 0xFF << 16 );
	LPC_SWM->PINASSIGN0 = regVal | ( 12 << 16 );			/* P0.12 is UART0 RTS, ASSIGN0(23:16) */
	regVal = LPC_SWM->PINASSIGN0 & ~( 0xFFUL << 24 );
	LPC_SWM->PINASSIGN0 = regVal | ( 13 << 24 );			/* P0.13 is UART0 CTS. ASSIGN0(31:24) */
#endif

#if 0
  /* connect the UART1 TXD abd RXD sigals to port pins(P0.4-P0.0)*/
	regVal = LPC_SWM->PINASSIGN1 & ~( 0xFF << 8 );
	LPC_SWM->PINASSIGN1 = regVal | ( 4 << 8 );				/* P0.4 is UART1 TXD, ASSIGN1(15:8) */
	regVal = LPC_SWM->PINASSIGN1 & ~( 0xFF << 16 );
	LPC_SWM->PINASSIGN1 = regVal | ( 0 << 16 );			/* P0.0 is UART1 RXD. ASSIGN1(23:16) */
	
	regVal = LPC_SWM->PINASSIGN1 & ~( 0xFFUL << 24 );
	LPC_SWM->PINASSIGN1 = regVal | ( 12 << 24 );			/* P0.12 is UART1 RTS. ASSIGN1(31:24) */
	regVal = LPC_SWM->PINASSIGN2 & ~( 0xFF << 0 );
	LPC_SWM->PINASSIGN2 = regVal | ( 13 << 0 );			/* P0.13 is UART1 RTS, ASSIGN2(7:0) */
#endif

#if 0
	/* connect the UART2 TXD abd RXD sigals to port pins(P0.4-P0.0)*/
	regVal = LPC_SWM->PINASSIGN2 & ~( 0xFF << 16 );
	LPC_SWM->PINASSIGN2 = regVal | ( 4 << 16 );			/* P0.4 is UART2 TXD, ASSIGN2(23:16) */
	regVal = LPC_SWM->PINASSIGN2 & ~( 0xFFUL << 24 );
	LPC_SWM->PINASSIGN2 = regVal | ( 0 << 24 );			/* P0.0 is UART2 RXD. ASSIGN2(31:24) */
	
	regVal = LPC_SWM->PINASSIGN3 & ~( 0xFF << 0 );
	LPC_SWM->PINASSIGN3 = regVal | ( 12 << 0 );			/* P0.12 is UART2 RTS. ASSIGN1(7:0) */
	regVal = LPC_SWM->PINASSIGN3 & ~( 0xFF << 8 );
	LPC_SWM->PINASSIGN3 = regVal | ( 13 << 8 );			/* P0.13 is UART2 RTS, ASSIGN2(15:8) */
#endif

//  UARTInit(LPC_USART0, 115200);
	UARTInit(LPC_USART0, 9600);

#if LOOPBACK_TEST
  LoopbackTest(LPC_USART0);
#else
	UARTSend(LPC_USART0, (uint8_t *)"Hello World!\r\n", 14);
	for ( i = 0; i < 0x10000; i++ );
  
	while ( 1 )
  {
		if ( UARTRxCount && !RxErrorCount )
		{
			LPC_USART0->INTENCLR = RXRDY;	/* Disable RXRDY */
			UARTSend( LPC_USART0, (uint8_t *)UARTRxBuffer, UARTRxCount );
			UARTRxCount = 0;
			LPC_USART0->INTENSET = RXRDY;	/* Re-enable RXRDY */
		}
  }
#endif
}
