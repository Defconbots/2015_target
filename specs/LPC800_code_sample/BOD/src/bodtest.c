/****************************************************************************
 *   $Id:: bodtest.c 4309 2012-10-31 01:02:28Z usb00423                     $
 *   Project: NXP LPC8xx BOD(Brown-OUt Detect) example
 *
 *   Description:
 *     This file contains BOD code example which tests BOD 
 *     initialization, BOD interrupt handler, and APIs.
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
#include "lpc8xx_gpio.h"
#include "lpc8xx_mrt.h"
#include "lpc8xx_bod.h"
#if DEEPSLEEP_BOD_WAKEUP
#include "lpc8xx_pmu.h"
#endif

extern volatile uint32_t bod_counter;
extern uint32_t mrt_counter;

/*****************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void)
{
#if DEEPSLEEP_BOD_WAKEUP
  uint32_t regVal;
#endif

  SystemCoreClockUpdate();

  /* Initialize BOD */
  BOD_Init();

  /* Set port 0_0, 0_1, and 0_2 to output.
  0_0 is for BOD reset,  0.1 is for POR reset, 0.2 is for BOD interrupt. */
  GPIOInit();

  GPIOSetDir( 0, BOD_RESET_LED, 1 );
  GPIOSetDir( 0, POR_RESET_LED, 1 );
  GPIOSetDir( 0, BOD_INTERRUPT_LED, 1 );
#if DEEPSLEEP_BOD_WAKEUP
  /* when external event(p0.3) to trigger the power down. */
  GPIOSetDir( 0, PWR_DOWN_BUTTON, 0 );
#endif

  /* Turn off all LEDs. */
  GPIOSetBitValue( 0, BOD_RESET_LED, 0 );
  GPIOSetBitValue( 0, POR_RESET_LED, 0 );
  GPIOSetBitValue( 0, BOD_INTERRUPT_LED, 0 );

	init_mrt(0x2000);

  /* The procedure is:
  (1) POR LED should blink on after power cycle.
  (2) when voltage(VDD) falls to the BOD interrupt level, BOD Interrupt
  (p0.2) LED will be ON, POR LED(p0.1) should be off. 
  (3) when voltage raises, BOD interrupt(p0.2) LED should be off, POR RESET 
  (p0.1) LED will blink back on again. 
  (4) when voltage drops further to the BOD RESET, BOD RESET (p0.0) LED will 
  blink. 
  (5) Once it happens, BOD interrupt is disabled. */
   
  /* Enable the BOD Interrupt */
  NVIC_EnableIRQ(BOD_IRQn);

  if ((LPC_SYSCON->SYSRSTSTAT & (POR_RESET|BOD_RESET)) == BOD_RESET)
  {
    LPC_SYSCON->SYSRSTSTAT |= BOD_RESET;
	/* Blink port 1_0 */
    while( 1 ) 
    {
			if ( GPIOGetPinValue( 0, BOD_INTERRUPT_LED ) )
			{
				/* if BOD interrupt has occured and LED is on, turn it off. */
				GPIOSetBitValue( 0, BOD_INTERRUPT_LED, 0 );
			}
			if ( (mrt_counter > 0) && (mrt_counter <= 200) )
			{
				GPIOSetBitValue( 0, BOD_RESET_LED, 0 );
			}
			if ( (mrt_counter > 200) && (mrt_counter <= 400) )
			{
				GPIOSetBitValue( 0, BOD_RESET_LED, 1 );
			}
			else if ( mrt_counter > 400 )
			{
				mrt_counter = 0;
			}
    }
  }
  else
  {
    LPC_SYSCON->SYSRSTSTAT |= POR_RESET;
	/* Blink port 0_1 */
    while( 1 ) 
    {
#if DEEPSLEEP_BOD_WAKEUP
			/* If DEEPSLEEP_BOD_WAKEUP flag is true. It will test, 
			when MCU is in deep sleep mode, BOD RESET can be used 
			as the wakeup source. 
			The procedure is: 
			(1) Install jumper to put MCU into deep sleep mode.
			(2) Decrease the VDD voltage below BOD RESET.
			(3) Increase the VDD voltage back to BOD RESET range.
			(4) The system will be waken up, and BOD RESET LED
			should blink. */
			if ( !GPIOGetPinValue( 0, PWR_DOWN_BUTTON ) )
			{
				regVal = LPC_SYSCON->PDSLEEPCFG;
				regVal &= ~(WDT_OSC_PD | BOD_PD);
				PMU_Sleep( MCU_DEEP_SLEEP, regVal );
			}
#endif	  
			if ( GPIOGetPinValue( 0, BOD_INTERRUPT_LED ) )
			{
				/* if BOD interrupt has occured and LED is on, turn it off. */
				GPIOSetBitValue( 0, BOD_INTERRUPT_LED, 0 );
			}
			/* Toggle p1.1 indicating it's a POR reset */
			if ( (mrt_counter > 0) && (mrt_counter <= 200) )
			{
				GPIOSetBitValue( 0, POR_RESET_LED, 0 );
			}
			if ( (mrt_counter > 200) && (mrt_counter <= 400) )
			{
				GPIOSetBitValue( 0, POR_RESET_LED, 1 );
			}
			else if ( mrt_counter > 400 )
			{
				mrt_counter = 0;
			}
    }
  }
}

/*********************************************************************************
**                            End Of File
*********************************************************************************/
