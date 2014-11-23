/****************************************************************************
 *   $Id:: spitest.c 3635 2012-10-31 00:31:46Z usb00423                     $
 *   Project: NXP LPC8xx SPI example
 *
 *   Description:
 *     This file contains SSP test modules, main entry, to test SSP APIs.
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
#include "LPC8xx.h"                        /* LPC8xx definitions */
#include "lpc8xx_spi.h"
		
volatile uint8_t src_addr[SPI_BUFSIZE]; 
volatile uint8_t dest_addr[SPI_BUFSIZE];

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
	 * Initialize SPI pin connect
	 */
#if 1
  /*connect the SPI0 SSEL, SCK MOSI, and MISO sigals to port pins(P0.6-P0.8, p0.14)*/
	regVal = LPC_SWM->PINASSIGN3 & ~( 0xFFUL<<24 );
	LPC_SWM->PINASSIGN3 = regVal | ( 0x6UL<<24 );	/* P0.6 is SCK, ASSIGN3(31:24) */ 
	regVal = LPC_SWM->PINASSIGN4 & ~( 0xFF<<0 );
	LPC_SWM->PINASSIGN4 = regVal | ( 0x7<<0 );			/* P0.7 is MOSI. ASSIGN4(7:0) */
	regVal = LPC_SWM->PINASSIGN4 & ~( 0xFF<<8 );
	LPC_SWM->PINASSIGN4 = regVal | ( 0x8<<8 );			/* P0.8 is MISO. ASSIGN4(15:8) */
	regVal = LPC_SWM->PINASSIGN4 & ~( 0xFF<<16 );
	LPC_SWM->PINASSIGN4 = regVal | ( 14<<16 );			/* P0.14 is SSEL. ASSIGN4(23:16) */
#else
  /*connect the SPI1 SSEL, SCK MOSI, and MISO sigals to port pins(P0.6-P0.8, p0.14)*/
	regVal = LPC_SWM->PINASSIGN4 & ~( 0xFFUL<<24 );
	LPC_SWM->PINASSIGN4 = regVal | ( 0x6UL<<24 );	/* P0.6 is SCK, ASSIGN3(31:24) */ 
	regVal = LPC_SWM->PINASSIGN5 & ~( 0xFF<<0 );
	LPC_SWM->PINASSIGN5 = regVal | ( 0x7<<0 );			/* P0.7 is MOSI. ASSIGN4(7:0) */
	regVal = LPC_SWM->PINASSIGN5 & ~( 0xFF<<8 );
	LPC_SWM->PINASSIGN5 = regVal | ( 0x8<<8 );			/* P0.8 is MISO. ASSIGN4(15:8) */
	regVal = LPC_SWM->PINASSIGN5 & ~( 0xFF<<16 );
	LPC_SWM->PINASSIGN5 = regVal | ( 14<<16 );			/* P0.14 is SSEL. ASSIGN4(23:16) */
#endif

//  SPI_Init(LPC_SPI0, 0xFFFF, CFG_CPHA | CFG_CPOL, 0x0);			
  SPI_Init(LPC_SPI0, 0xFFFF, CFG_SLAVE, DLY_PREDELAY(0)|DLY_POSTDELAY(0)|DLY_FRAMEDELAY(0)|DLY_INTERDELAY(0));
    						
  for ( i = 0; i < SPI_BUFSIZE; i++ )
  {
		src_addr[i] = (uint8_t)i;
		dest_addr[i] = 0;
  }

  /* For the inter-board communication, one board is set as
  master transmit, the other is set to slave receive. */
  /* Slave receive */
  SPI_SlaveReceive( LPC_SPI0, (uint8_t *)dest_addr, SPI_BUFSIZE );
  for ( i = 0; i < SPI_BUFSIZE; i++ )
  {
		if ( src_addr[i] != dest_addr[i] )
		{
			while ( 1 );				/* Verification failure, fatal error */
		} 
  }
	
  LPC_SPI0->TXCTRL = TXDATCTL_RX_IGNORE;
  SPI_SlaveSend( LPC_SPI0, (uint8_t *)src_addr, SPI_BUFSIZE );
  while ( 1 );

}

/******************************************************************************
**                            End Of File
******************************************************************************/

