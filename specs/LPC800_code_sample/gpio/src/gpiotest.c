/****************************************************************************
 *   $Id:: gpiotest.c 5400 2012-10-31 20:36:20Z usb00423                    $
 *   Project: NXP LPC8xx GPIO example
 *
 *   Description:
 *     This file contains GPIO test modules, main entry, to test GPIO APIs.
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
#include "lpc8xx_gpio.h"

#define TEST_DIGITAL_FILTER			0
#define TEST_PATTERN_MATCH			1

/*****************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void)
{
	volatile uint32_t regVal;
	
  SystemCoreClockUpdate();

  GPIOInit();

#if !TEST_PATTERN_MATCH
  /* Enable Hysteresis to eliminate debouncing. */
  LPC_IOCON->PIO0_0  |= (0x01<<5);	
  LPC_IOCON->PIO0_1  |= (0x01<<5);
  LPC_IOCON->PIO0_16 |= (0x01<<5);
  LPC_IOCON->PIO0_17 |= (0x01<<5);

	/* Route p0.0 to PINT0, p0.1 to PINT1, p0.16 to PINT2,
  and P0.17 to PINT3. */

  /* use port0_0 as input event, interrupt test. */
  GPIOSetDir( PORT0, 0, 0 );	
  /* channel 0, port0_0, edge trigger, active high. */
  GPIOSetPinInterrupt( CHANNEL0, PORT0, 0, 0, 1 );
  GPIOPinIntEnable( CHANNEL0, 1 );

  /* use port0_1 as input event, interrupt test. */
  GPIOSetDir( PORT0, 1, 0 );
  /* channel 1, port0_1, edge trigger, active high. */
  GPIOSetPinInterrupt( CHANNEL1, PORT0, 1, 0, 1 );
  GPIOPinIntEnable( CHANNEL1, 1 );

  /* use port0_16 as input event, interrupt test. */
  GPIOSetDir( PORT0, 16, 0 );
  /* channel 2, port0_16, edge trigger, active high. */
  GPIOSetPinInterrupt( CHANNEL2, PORT0, 16, 0, 1 );
  GPIOPinIntEnable( CHANNEL2, 1 );

  /* use port0_17 as input event, interrupt test. */
  GPIOSetDir( PORT0, 17, 0 );
  /* channel 3, port0_17, edge trigger, active high. */
  GPIOSetPinInterrupt( CHANNEL3, PORT0, 17, 0, 1 );
  GPIOPinIntEnable( CHANNEL3, 1 );

  /* Route p0.8 to PINT4, p0.9 to PINT5, p0.14 to PINT6,
  and P0.15 to PINT7. */
	/* Enable Hysteresis to eliminate debouncing. */
  LPC_IOCON->PIO0_8  |= (0x01<<5);	
  LPC_IOCON->PIO0_9  |= (0x01<<5);
  LPC_IOCON->PIO0_14 |= (0x01<<5);
  LPC_IOCON->PIO0_15 |= (0x01<<5);
  	 
  /* use port0_8 as input event, interrupt test. */
  GPIOSetDir( PORT0, 8, 0 );
	
	
#if TEST_DIGITAL_FILTER
	/* To test glitch filter, connect a signal generator
	generating 6MHz clock to P0_8, set combination of IOCONFILTRCLKDIV
	and sampling mode, bit 11~12, in IOCON such that <> 6MHz,
	an interrupt should be generated if filter doesn't filter out
	the glitch. */
	
	/* Enable IOCON clock divider 6. Note that IOCON divider registers
	IOCONFILTRCLKDIVx are in reversed order in SYSCON. */
	LPC_SYSCON->IOCONCLKDIV[DGF_CHANNEL6] = 0x2;
	regVal = LPC_IOCON->PIO0_8 & ~((0x3<<11)|(0x7<<13));
	/* S_MODE = 3, use IOCON filter clock div 6. */
	LPC_IOCON->PIO0_8 |= (regVal | (0x03<<11) | ((DGF_CHANNEL_NUM-DGF_CHANNEL6-1)<<13));
#endif

  /* channel 4, port0_8, level trigger, active low. */
  GPIOSetPinInterrupt( CHANNEL4, PORT0, 8, 1, 0 );
  GPIOPinIntEnable( CHANNEL4, 0 );

  /* use port0_9 as input event, interrupt test. */
  GPIOSetDir( PORT0, 9, 0 );
	
#if TEST_DIGITAL_FILTER
	/* Enable IOCON clock divider 0. Note that IOCON divider registers
	IOCONFILTRCLKDIVx are in reversed order in SYSCON. */
	LPC_SYSCON->IOCONCLKDIV[DGF_CHANNEL0] = 0x3;
	regVal = LPC_IOCON->PIO0_9 & ~((0x3<<11)|(0x7<<13));
	/* S_MODE = 3, use IOCON filter clock div 0. */
	LPC_IOCON->PIO0_9 |= (regVal | (0x03<<11) | ((DGF_CHANNEL_NUM-DGF_CHANNEL0-1)<<13));
#endif

  /* channel 5, port0_9, level trigger, active low. */
  GPIOSetPinInterrupt( CHANNEL5, PORT0, 9, 1, 0 );
  GPIOPinIntEnable( CHANNEL5, 0 );

  /* use port0_14 as input event, interrupt test. */
  GPIOSetDir( PORT0, 14, 0 );
	
#if TEST_DIGITAL_FILTER
	/* Enable IOCON clock divider 1. Note that IOCON divider registers
	IOCONFILTRCLKDIVx are in reversed order in SYSCON. */
	LPC_SYSCON->IOCONCLKDIV[DGF_CHANNEL1] = 0x4;
	regVal = LPC_IOCON->PIO0_14 & ~((0x3<<11)|(0x7<<13));
	/* S_MODE = 3, use IOCON filter clock div 1. */
	LPC_IOCON->PIO0_14 |= (regVal | (0x03<<11) | ((DGF_CHANNEL_NUM-DGF_CHANNEL1-1)<<13));
#endif

  /* channel 6, port0_14, level trigger, active low. */
  GPIOSetPinInterrupt( CHANNEL6, PORT0, 14, 1, 0 );
  GPIOPinIntEnable( CHANNEL6, 0 );

  /* use port0_15 as input event, interrupt test. */
  GPIOSetDir( PORT0, 15, 0 );
	
#if TEST_DIGITAL_FILTER
	/* Enable IOCON clock divider 5. Note that IOCON divider registers
	IOCONFILTRCLKDIVx are in reversed order in SYSCON. */
	LPC_SYSCON->IOCONCLKDIV[DGF_CHANNEL5] = 0x5;
	regVal = LPC_IOCON->PIO0_15 & ~((0x3<<11)|(0x7<<13));
	/* S_MODE = 2, use IOCON filter clock div 5. */
	LPC_IOCON->PIO0_15 |= (regVal | (0x02<<11) | ((DGF_CHANNEL_NUM-DGF_CHANNEL5-1)<<13));
#endif

  /* channel 7, port0_15, level trigger, active high. */
  GPIOSetPinInterrupt( CHANNEL7, PORT0, 15, 1, 0 );
  GPIOPinIntEnable( CHANNEL7, 1 );

#else

	/* Test Pattern match */
	GPIOPatternMatchInit();
	GPIOSetPatternMatchSlice( CHANNEL0, SLICE0, PATTERN_CONST_1, FALSE );		
	GPIOSetPatternMatchSlice( CHANNEL1, SLICE1, PATTERN_CONST_1, FALSE );		
	GPIOSetPatternMatchSlice( CHANNEL2, SLICE2, PATTERN_CONST_1, FALSE );
	GPIOSetPatternMatchSlice( CHANNEL3, SLICE3, PATTERN_CONST_1, FALSE );		
	GPIOSetPatternMatchSlice( CHANNEL4, SLICE4, PATTERN_CONST_1, FALSE );			
	
	GPIOSetPatternMatchInput( CHANNEL5, PORT0, 10 );
	GPIOSetPatternMatchSlice( CHANNEL5, SLICE5, PATTERN_LEVEL_LO, FALSE );		
	GPIOSetPatternMatchInput( CHANNEL6, PORT0, 11 );
	GPIOSetPatternMatchSlice( CHANNEL6, SLICE6, PATTERN_LEVEL_HI, FALSE );		
	GPIOSetPatternMatchInput( CHANNEL7, PORT0, 12 );
	GPIOSetPatternMatchSlice( CHANNEL7, SLICE7, PATTERN_RISING, TRUE );	
	
/* Config GPIO INT_BMAT, pattern match output. */
	regVal = LPC_SWM->PINASSIGN8 & ~( 0xFFUL << 24 );
	LPC_SWM->PINASSIGN8 = regVal | ( 13 << 24 );	/* P0.13 is pattern match output, ASSIGN(31:24). */
	LPC_IOCON->PIO0_13 &= ~(0x3<<3);		/* Change p0.13 from pull-up to inactive mode. */		
	
	GPIOPatternMatchEnable( TRUE, TRUE );
#endif

	while( 1 );
}

/*********************************************************************************
**                            End Of File
*********************************************************************************/
