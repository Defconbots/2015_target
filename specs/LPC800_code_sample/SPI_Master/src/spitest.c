/****************************************************************************
 *   $Id:: spitest.c 3635 2010-06-02 00:31:46Z usb00423                     $
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

#define CS_USED			SLAVE0
		
volatile uint8_t src_addr[SPI_BUFSIZE]; 
volatile uint8_t dest_addr[SPI_BUFSIZE];

/*****************************************************************************
** Function name:		SEEPROMTest
**
** Descriptions:		Serial EEPROM(Atmel 25xxx) test
**				
** parameters:			port #
** Returned value:		None
** 
*****************************************************************************/
void SPI_SEEPROMTest( LPC_SPI_TypeDef *SPIx, SLAVE_t slave )
{
  uint32_t i, timeout;

  /* Test Atmel AT25DF041 Serial flash. */
  src_addr[0] = WREN;			/* set write enable latch */
  SPI_Send( SPIx, slave, (uint8_t *)src_addr, 1 );
  for ( i = 0; i < 0x80; i++ );	/* delay minimum 250ns */

  src_addr[0] = RDSR;	/* check status to see if write enabled is latched */
  src_addr[1] = 0x55;	/* Dummy byte for read. */
  SPI_SendRcv( SPIx, slave, (uint8_t *)src_addr, (uint8_t *)dest_addr, 2 );
  if ( (dest_addr[1] & (RDSR_WEN|RDSR_RDY)) != RDSR_WEN )
  /* bit 0 to 0 is ready, bit 1 to 1 is write enable */
  {			    
		while ( 1 );
  }

  for ( i = 0; i < 0x80; i++ );	/* delay minimum 250ns */
  src_addr[0] = WRSR;
  src_addr[1] = 0x00;				/* Make the whole device unprotected. */
  SPI_Send( SPIx, slave, (uint8_t *)src_addr, 2 );

  for ( i = 0; i < 0x80; i++ );	/* delay minimum 250ns */
  src_addr[0] = RDSR;				/* check status to see if write enabled is latched */
  src_addr[1] = 0x55;	/* Dummy byte for read. */
  SPI_SendRcv( SPIx, slave, (uint8_t *)src_addr, (uint8_t *)dest_addr, 2 );
  if ( (dest_addr[1] & (RDSR_WEN|RDSR_RDY)) != RDSR_WEN )
  /* bit 0 to 0 is ready, bit 1 to 1 is write enable */
  {
		while ( 1 );
  }

  for ( i = 0; i < 0x80; i++ );	/* delay minimum 250ns */
  src_addr[0] = CHIP_ERASE;	/* Write command is 0x02, low 256 bytes only */
  SPI_Send( SPIx, slave, (uint8_t *)src_addr, 1 );

  for ( i = 0; i < 0x1400000; i++ );	/* Be careful with the dumb delay, it
										may vary depending on the system clock.  */
  src_addr[0] = RDSR;	/* check status to see if write enabled is latched */
  src_addr[1] = 0x55;	/* Dummy byte for read. */	
  SPI_SendRcv( SPIx, slave, (uint8_t *)src_addr, (uint8_t *)dest_addr, 2 );
  if ( (dest_addr[1] & (RDSR_EPE|RDSR_RDY)) != 0x0 )
  /* bit 0 to 0 is ready, bit 1 to 1 is write enable */
  {
		while ( 1 );
  }

  /* Test Atmel AT25DF041 Serial flash. */
  src_addr[0] = WREN;			/* set write enable latch */
  SPI_Send( SPIx, slave, (uint8_t *)src_addr, 1 );

  for ( i = 0; i < 0x80; i++ );	/* delay minimum 250ns */
  src_addr[0] = RDSR;	/* check status to see if write enabled is latched */
  src_addr[1] = 0x55;	/* Dummy byte for read. */	
  SPI_SendRcv( SPIx, slave, (uint8_t *)src_addr, (uint8_t *)dest_addr, 2 );
  if ( (dest_addr[1] & (RDSR_WEN|RDSR_RDY)) != RDSR_WEN )
  /* bit 0 to 0 is ready, bit 1 to 1 is write enable */
  {
		while ( 1 );
  }

  for ( i = 0; i < SPI_BUFSIZE; i++ )	/* Init RD and WR buffer */    
  {
		src_addr[i+SFLASH_INDEX] = i;	/* leave four bytes for cmd and offset(16 bits) */
		dest_addr[i] = 0;
  }

  /* please note the first four bytes of WR and RD buffer is used for
  commands and offset, so only 4 through SSP_BUFSIZE is used for data read,
  write, and comparison. */
  src_addr[0] = WRITE;	/* Write command is 0x02, low 256 bytes only */
  src_addr[1] = 0x00;	/* write address offset MSB is 0x00 */
  src_addr[2] = 0x00;	/* write address offset LSB is 0x00 */
  src_addr[3] = 0x00;	/* write address offset LSB is 0x00 */
  SPI_Send( SPIx, slave, (uint8_t *)src_addr, SPI_BUFSIZE );

  for ( i = 0; i < 0x400000; i++ );	/* Be careful with the dumb delay, it
										may vary depending on the system clock.  */
  timeout = 0;
  while ( timeout < MAX_TIMEOUT )
  {
		src_addr[0] = RDSR;	/* check status to see if write cycle is done or not */
		src_addr[1] = 0x55;	/* Dummy byte for read. */	
		SPI_SendRcv( SPIx, slave, (uint8_t *)src_addr, (uint8_t *)dest_addr, 2 );
		if ( (dest_addr[1] & RDSR_RDY) == 0x00 )	/* bit 0 to 0 is ready */
		{
			break;
		}
		timeout++;
  }
  if ( timeout == MAX_TIMEOUT )
  {
		while ( 1 );
  }

  for ( i = 0; i < 0x80; i++ );	/* delay, minimum 250ns */
  src_addr[0] = READ;		/* Read command is 0x03, low 256 bytes only */
  src_addr[1] = 0x00;		/* Read address offset MSB is 0x00 */
  src_addr[2] = 0x00;		/* Read address offset LSB is 0x00 */
  src_addr[3] = 0x00;		/* Read address offset LSB is 0x00 */
  SPI_SendRcv( SPIx, slave, (uint8_t *)src_addr, (uint8_t *)&dest_addr, SPI_BUFSIZE );
  return;
}

/*****************************************************************************
** Function name:		SPI_Loopback
**
** Descriptions:		Loopback test
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void SPI_Loopback( LPC_SPI_TypeDef *SPIx )
{
  uint32_t i;

  SPIx->CFG |= CFG_LOOPBACK;

  for ( i = 0; i < SPI_BUFSIZE; i++ )
  {
		src_addr[i] = (uint8_t)i;
		dest_addr[i] = 0;
  }
  
  i = 0;
  while ( i < SPI_BUFSIZE ) {
		/* Move only if TXRDY is ready */
		while ( (SPIx->STAT & STAT_TXRDY) == 0 );
		/* Set frame length to fixed 8 for now. */
		if ( i == 0 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(CS_USED) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | src_addr[i];
		}
		else if ( i == SPI_BUFSIZE-1 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(CS_USED) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | TXDATCTL_EOT | src_addr[i];
		}
		else {
			SPIx->TXDAT = src_addr[i];
		}
		while ( (SPIx->STAT & STAT_RXRDY) == 0 );
		dest_addr[i] = SPIx->RXDAT;
		i++;
  }

  /* Restore CFG register. */  
  SPIx->CFG &= ~CFG_LOOPBACK;
  
  for ( i = 0; i < SPI_BUFSIZE; i++ )
  {
		if ( src_addr[i] != dest_addr[i] )
		{
			while( 1 );			/* Verification failed */
		}
  }
  return; 
  
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
  uint32_t i;
  //regVal;

	SystemCoreClockUpdate();
	
	/*
	 * Initialize SPI pin connect
	 */
#if 1
  /*connect the SPI0 SSEL, SCK MOSI, and MISO sigals to port pins(P0.6-P0.8, p0.14)*/
//	regVal = LPC_SWM->PINASSIGN3 & ~( 0xFFUL<<24 );
//	LPC_SWM->PINASSIGN3 = regVal | ( 0x6UL<<24 );	/* P0.6 is SCK, ASSIGN3(31:24) */ 
//	regVal = LPC_SWM->PINASSIGN4 & ~( 0xFF<<0 );
//	LPC_SWM->PINASSIGN4 = regVal | ( 0x7<<0 );			/* P0.7 is MOSI. ASSIGN4(7:0) */
//	regVal = LPC_SWM->PINASSIGN4 & ~( 0xFF<<8 );
//	LPC_SWM->PINASSIGN4 = regVal | ( 0x8<<8 );			/* P0.8 is MISO. ASSIGN4(15:8) */
//	regVal = LPC_SWM->PINASSIGN4 & ~( 0xFF<<16 );
//	LPC_SWM->PINASSIGN4 = regVal | ( 14<<16 );			/* P0.14 is SSEL. ASSIGN4(23:16) */

    /* Pin Assign 8 bit Configuration */
    /* SPI0_SCK */
    LPC_SWM->PINASSIGN3 = 0x0cffffffUL; 
    /* SPI0_MOSI */
    /* SPI0_MISO */
    /* SPI0_SSEL */
    LPC_SWM->PINASSIGN4 = 0xff0f0e0dUL; 

#else
  /*connect the SPI1 SSEL, SCK MOSI, and MISO sigals to port pins(P0.6-P0.8, p0.14)*/
//	regVal = LPC_SWM->PINASSIGN4 & ~( 0xFFUL<<24 );
//	LPC_SWM->PINASSIGN4 = regVal | ( 0x6UL<<24 );	/* P0.6 is SCK, ASSIGN3(31:24) */ 
//	regVal = LPC_SWM->PINASSIGN5 & ~( 0xFF<<0 );
//	LPC_SWM->PINASSIGN5 = regVal | ( 0x7<<0 );			/* P0.7 is MOSI. ASSIGN4(7:0) */
//	regVal = LPC_SWM->PINASSIGN5 & ~( 0xFF<<8 );
//	LPC_SWM->PINASSIGN5 = regVal | ( 0x8<<8 );			/* P0.8 is MISO. ASSIGN4(15:8) */
//	regVal = LPC_SWM->PINASSIGN5 & ~( 0xFF<<16 );
//	LPC_SWM->PINASSIGN5 = regVal | ( 14<<16 );			/* P0.14 is SSEL. ASSIGN4(23:16) */
#endif

  SPI_Init(LPC_SPI0, 0x05, CFG_MASTER, DLY_PREDELAY(0x0)|DLY_POSTDELAY(0x0)|DLY_FRAMEDELAY(0x0)|DLY_INTERDELAY(0x0));

#if SPI_LOOPBACK_TEST
  SPI_Loopback( LPC_SPI0 );
#else			
  for ( i = 0; i < SPI_BUFSIZE; i++ )
  {
		src_addr[i] = (uint8_t)i;
		dest_addr[i] = 0;
  }

#if SPI_TX_RX
  /* For the inter-board communication, one board is set as
  master transmit, the other is set to slave receive. */
  /* Master transmit */
  SPI_Send( LPC_SPI0, CS_USED, (uint8_t *)src_addr, SPI_BUFSIZE);
  SPI_Receive( LPC_SPI0, CS_USED, (uint8_t *)dest_addr, SPI_BUFSIZE);
  for ( i = 0; i < SPI_BUFSIZE; i++ )
  {
		if ( src_addr[i] != dest_addr[i] )
		{
			while( 1 );			/* Verification failed */
		}
  }
#else
  /* SPI_TX_RX=0, it's to communicate with a serial EEPROM. */
  SPI_SEEPROMTest(LPC_SPI0, CS_USED);  

  /* for EEPROM test, verifying, ignore the difference in the first 
  four bytes which are used as command and parameters. */
  for ( i = SFLASH_INDEX; i < SPI_BUFSIZE; i++ )
  {
		if ( src_addr[i] != dest_addr[i] )
		{
			while( 1 );			/* Verification failed */
		}
  }
#endif			/* endif NOT TX_RX_ONLY */
#endif			/* endif SPI_LOOPBACK_TEST */
  while ( 1 );

}

/******************************************************************************
**                            End Of File
******************************************************************************/

