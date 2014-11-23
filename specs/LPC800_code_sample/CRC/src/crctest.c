/****************************************************************************
 *   $Id:: crctest.c 8351 2012-10-31 21:56:56Z usb00423                     $
 *   Project: NXP LPC8xx CRC example
 *
 *   Description:
 *     This file contains CRC test modules, main entry, to test CRC APIs.
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
#include "stdint.h"
#include "lpc8xx_crc.h"

/* Main Program */
volatile uint32_t regVal;
  
int main (void) {

  SystemCoreClockUpdate();

	/* Enable CRC clock */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<13);
	
  while (1)                                /* Loop forever */
  {
		/* CRC-CCITT 8 bits */
		CRC_SetMode(CRC_CCITT_POLYNOMIAL, 0, 0, 0, 0);
		LPC_CRC->SEED = 0xFFFF;
		regVal = 0x38;
		CRC_WriteData((uint8_t *)&regVal, 1);
		regVal = CRC_GetSum(); // 0x56AB expected

		regVal = 0x38;
		CRC_WriteData((uint8_t *)&regVal, 1);
		regVal = CRC_GetSum(); // 0x2668 expected

		regVal = 0x38;
		CRC_WriteData((uint8_t *)&regVal, 1);
		regVal = CRC_GetSum(); // 0x9BFF expected

		regVal = 0x38;
		CRC_WriteData((uint8_t *)&regVal, 1);
		regVal = CRC_GetSum(); // 0x7A89 expected

		CRC_SetMode(CRC_CCITT_POLYNOMIAL, 1, 0, 0, 0);
		LPC_CRC->SEED = 0xFFFF;
		regVal = 0x38;
		CRC_WriteData((uint8_t *)&regVal, 1);
		regVal = CRC_GetSum(); // 0x324D expected

		CRC_SetMode(CRC_CCITT_POLYNOMIAL, 0, 0, 1, 0);
		LPC_CRC->SEED = 0xFFFF;
		regVal = 0x38;
		CRC_WriteData((uint8_t *)&regVal, 1);
		regVal = CRC_GetSum(); // 0xD56A expected

		CRC_SetMode(CRC_CCITT_POLYNOMIAL, 1, 0, 1, 0);
		LPC_CRC->SEED = 0xFFFF;
		regVal = 0x38;
		CRC_WriteData((uint8_t *)&regVal, 1);
		regVal = CRC_GetSum(); // 0xB24C expected

    /* CRC-CCITT 16 bits */
		CRC_SetMode(CRC_CCITT_POLYNOMIAL, 0, 0, 0, 0);
		LPC_CRC->SEED = 0xFFFF;
		regVal = 0x3736;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0xF488 expected

		regVal = 0x3736;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0x2602 expected

		regVal = 0x3736;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0x6585 expected

		regVal = 0x3736;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0xDC75 expected

		CRC_SetMode(CRC_CCITT_POLYNOMIAL, 1, 0, 0, 0);
		LPC_CRC->SEED = 0xFFFF;
		regVal = 0x3736;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0x6FEA expected

		CRC_SetMode(CRC_CCITT_POLYNOMIAL, 0, 0, 1, 0);
		LPC_CRC->SEED = 0xFFFF;
		regVal = 0x3736;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0x112F expected

		CRC_SetMode(CRC_CCITT_POLYNOMIAL, 1, 0, 1, 0);
		LPC_CRC->SEED = 0xFFFF;
		regVal = 0x3736;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0x57F6 expected

    /* CRC-16 16 bits */
		CRC_SetMode(CRC_16_POLYNOMIAL, 0, 0, 0, 0);
		LPC_CRC->SEED = 0x0000;
		regVal = 0x3534;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0x38BD expected

		CRC_SetMode(CRC_16_POLYNOMIAL, 1, 0, 0, 0);
		LPC_CRC->SEED = 0x0000;
		regVal = 0x3534;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0x6BEB expected

		CRC_SetMode(CRC_16_POLYNOMIAL, 0, 0, 1, 0);
		LPC_CRC->SEED = 0x0000;
		regVal = 0x3534;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0xBD1C expected

		CRC_SetMode(CRC_16_POLYNOMIAL, 1, 0, 1, 0);
		LPC_CRC->SEED = 0x0000;
		regVal = 0x3534;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0xD7D6 expected

		regVal = 0x3534;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0xE9C8 expected

		regVal = 0x3534;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0x9940 expected

		regVal = 0x3534;
		CRC_WriteData((uint8_t *)&regVal, 2);
		regVal = CRC_GetSum(); // 0x7D27 expected

    /* CRC-32 32 bits */
		CRC_SetMode(CRC_32_POLYNOMIAL, 0, 0, 0, 1);
		LPC_CRC->SEED = 0xFFFFFFFF;
		regVal = 0x33323130;
		CRC_WriteData((uint8_t *)&regVal, 4);
		regVal = CRC_GetSum(); // 0x4B30C5B2 expected

		CRC_SetMode(CRC_32_POLYNOMIAL, 1, 0, 0, 1);
		LPC_CRC->SEED = 0xFFFFFFFF;
		regVal = 0x33323130;
		CRC_WriteData((uint8_t *)&regVal, 4);
		regVal = CRC_GetSum(); // 0xBEB96665 expected

		CRC_SetMode(CRC_32_POLYNOMIAL, 0, 0, 1, 1);
		LPC_CRC->SEED = 0xFFFFFFFF;
		regVal = 0x33323130;
		CRC_WriteData((uint8_t *)&regVal, 4);
		regVal = CRC_GetSum(); // 0x4DA30CD2 expected

		CRC_SetMode(CRC_32_POLYNOMIAL, 1, 0, 1, 1);
		LPC_CRC->SEED = 0xFFFFFFFF;
		regVal = 0x33323130;
		CRC_WriteData((uint8_t *)&regVal, 4);
		regVal = CRC_GetSum(); // 0xA6669D7D expected

		regVal = 0x33323130;
		CRC_WriteData((uint8_t *)&regVal, 4);
		regVal = CRC_GetSum(); // 0xC6EA0463 expected

		regVal = 0x33323130;
		CRC_WriteData((uint8_t *)&regVal, 4);
		regVal = CRC_GetSum(); // 0x5E6A11AF expected

		regVal = 0x33323130;
		CRC_WriteData((uint8_t *)&regVal, 4);
		regVal = CRC_GetSum(); // 0xBD4BA2B7 expected
  }
}
