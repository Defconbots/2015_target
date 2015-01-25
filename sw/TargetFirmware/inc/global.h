#ifndef GLOBAL_H
#define GLOBAL_H

#include <cr_section_macros.h>
#include "chip.h"
#include <stdio.h>

//#define CONTROLLER_TEST_SERIAL
//#define TARGET_TEST_SERIAL

//#define CONTROLLER
#if !defined(CONTROLLER)
#define TARGET
#endif

#if defined(CONTROLLER)
#include "controller.h"
#define MY_ADDRESS '0'
#endif

#if defined(TARGET)
#include "target.h"
#define MY_ADDRESS '1'
#endif

#define SUCCESS 0
#define FAILURE -1

#define _BV(x) (1<<(x))

// What is it going to return to O_o?
#pragma GCC diagnostic ignored "-Wmain"


#endif
