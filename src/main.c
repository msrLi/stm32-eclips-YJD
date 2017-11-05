/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include  <cpu.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <os.h>
#include  <os_app_hooks.h>

// #include  <app_cfg.h>

#include "Timer.h"
#include "BlinkLed.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F4 led blink sample (trace via DEBUG).
//
// In debug configurations, demonstrate how to print a greeting message
// on the trace device. In release configurations the message is
// simply discarded.
//
// Then demonstrates how to blink a led with 1 Hz, using a
// continuous loop and SysTick delays.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- Timing definitions -------------------------------------------------

// Keep the LED on for 2/3 of a second.
#define BLINK_ON_TICKS  (TIMER_FREQUENCY_HZ * 3 / 4)
#define BLINK_OFF_TICKS (TIMER_FREQUENCY_HZ - BLINK_ON_TICKS)

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#define APP_CFG_TASK_START_STK_SIZE 256u
#define  APP_CFG_TASK_START_PRIO                2u
                                                                /* --------------- APPLICATION GLOBALS ---------------- */
static  OS_TCB       AppTaskStartTCB;
static  CPU_STK      AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  void  AppTaskStart (void *p_arg)
{
	OS_ERR      err;
	uint32_t seconds = 0;
	(void)p_arg;

	CPU_Init();
	while(1)
	{
	      blink_led_on();
	      OSTimeDlyHMSM( 0u, 0u, 1u, 10u,
	      	                         OS_OPT_TIME_HMSM_STRICT,
	      	                        &err);
	      blink_led_off();
	      OSTimeDlyHMSM( 0u, 0u, 1u, 10u,
	      	                         OS_OPT_TIME_HMSM_STRICT,
	      	                        &err);
	      ++seconds;
	      // Count seconds on the trace device.
	      trace_printf("Second %u\n", seconds);
	}
}
int
main(int argc, char* argv[])
{
    OS_ERR   err;
#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR  cpu_err;
#endif
  // Send a greeting to the trace device (skipped on Release).
  trace_puts("Hello ARM World!");

  // At this stage the system clock should have already been configured
  // at high speed.
  trace_printf("System clock: %u Hz\n", SystemCoreClock);

  timer_start();

  blink_led_init();
  Mem_Init();                                                 /* Initialize Memory Managment Module                   */
  Math_Init();                                                /* Initialize Mathematical Module                       */
#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_NameSet((CPU_CHAR *)"STM32F407IG",
                (CPU_ERR  *)&cpu_err);
#endif
  /* disable all interrupt */
  // CPU_IntDis();
  OSInit(&err);                                               /* Init uC/OS-III.                                      */
  App_OS_SetAllHooks();

  OSTaskCreate(&AppTaskStartTCB,                              /* Create the start task                                */
                "App Task Start",
                AppTaskStart,
                0u,
                APP_CFG_TASK_START_PRIO,
               &AppTaskStartStk[0u],
                AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10u],
                APP_CFG_TASK_START_STK_SIZE,
                0u,
                0u,
                0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &err);

  OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
  // Infinite loop
  while (1)
    {
	  OSTimeDlyHMSM( 0u, 0u, 0u, 10u,
	                         OS_OPT_TIME_HMSM_STRICT,
	                        &err);
    }
  // Infinite loop, never return.
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
