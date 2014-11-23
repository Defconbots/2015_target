#ifndef __THE_REAL_SCT_USER_H__
#define __THE_REAL_SCT_USER_H__

#include "LPC8xx.h"

/** Macro to define register bits and mask in CMSIS style */
#define SCT_DefineRegBit(name,pos,width) \
    enum { \
        name##_Pos = pos, \
        name##_Msk = (int)(((1ul << width) - 1) << pos), \
    }


/* Declarations of SCT register fields */
SCT_DefineRegBit(SCT_CTRL_U_DOWN_L,					0,  1);
SCT_DefineRegBit(SCT_CTRL_U_STOP_L,					1,  1);
SCT_DefineRegBit(SCT_CTRL_U_HALT_L,					2,  1);
SCT_DefineRegBit(SCT_CTRL_U_CLRCTR_L,				3,  1);
SCT_DefineRegBit(SCT_CTRL_U_BIDIR_L,				4,  1);
SCT_DefineRegBit(SCT_CTRL_U_PRE_L,					5,  8);
SCT_DefineRegBit(SCT_CTRL_U_DOWN_H,					16, 1);
SCT_DefineRegBit(SCT_CTRL_U_STOP_H,					17, 1);
SCT_DefineRegBit(SCT_CTRL_U_HALT_H,					18, 1);
SCT_DefineRegBit(SCT_CTRL_U_CLRCTR_H,				19, 1);
SCT_DefineRegBit(SCT_CTRL_U_BIDIR_H,				20, 1);
SCT_DefineRegBit(SCT_CTRL_U_PRE_H,					21, 8);



/* Define expected signal characteristics */
#define PWM_FREQUENCY			10000		/* PWM frequency in Hz */
#define PWM_RESOLUTION_NS		1000		/* Timer resolution in ns */
#define PWM_MIN_DUTY_PERCENT	25			/* Minimum allowed duty cycle in % */
#define PWM_MAX_DUTY_PERCENT	70			/* Maximum allowed duty cycle in % */


/* Derived constants */
#define SCT_PRESCALER			(((SystemCoreClock / 1000u) * (PWM_RESOLUTION_NS)) / 1000000u - 1u)

#define min_pulse_width			((10000000u * PWM_MIN_DUTY_PERCENT) / (PWM_FREQUENCY * PWM_RESOLUTION_NS))
#define max_pulse_width			((10000000u * PWM_MAX_DUTY_PERCENT) / (PWM_FREQUENCY * PWM_RESOLUTION_NS))
#define timeout_value			((10000000u * 300                 ) / (PWM_FREQUENCY * PWM_RESOLUTION_NS))

#endif
