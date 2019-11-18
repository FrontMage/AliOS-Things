/*
 ** ###################################################################
 **     Processors:          GAP8
 **
 **     Compilers:           GNU C Compiler
 **
 **     Reference manual:    riscv-spec-v2.1, January 2017
 **     Version:             rev. 2.9, 2017-07-19
 **
 **     Abstract:
 **         CPU side bringup functions
 **
 **     Copyright (c) 2015 - 2019 GreenWave Technologies, Inc.
 **     All rights reserved.
 **
 **     Redistribution and use in source and binary forms, with or without modification,
 **     are permitted provided that the following conditions are met:
 **
 **     o Redistributions of source code must retain the above copyright notice, this list
 **       of conditions and the following disclaimer.
 **
 **     o Redistributions in binary form must reproduce the above copyright notice, this
 **       list of conditions and the following disclaimer in the documentation and/or
 **       other materials provided with the distribution.
 **
 **     o Neither the name of GreenWaves Technologies, Inc. nor the names of its
 **       contributors may be used to endorse or promote products derived from this
 **       software without specific prior written permission.
 **
 **     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 **     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 **     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 **     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 **     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 **     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 **     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 **     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 **     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 **     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **
 **     http:                 http://greenwaves-technologies.com
 **     mail:                 antoine.faravelon@greenwaves-technologies.com
                              
 **
 **     Revisions:
 **     - rev. 1.0 (2017-10-11)
 **         Initial version.
 **     - rev. 1.1 (2019-05-30)
 **         Re adapt for AliOS
 **     - rev. 1.2 (2019-10-11)
 **         Various clean ups, and remove all ref to old bridge
 ** ###################################################################
 */

/*!
 * @file system_GAP8.c
 * @version 1.1
 * @date 2019-05-XX
 * @brief Device specific configuration file for GAP8 (implementation file)
 *
 * Provides a system configuration function and a global variable that contains
 * the system frequency. It configures the device and initializes the oscillator
 * (PLL) that is part of the microcontroller device.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "system_gap8.h"
#include "pmsis.h"

/* ----------------------------------------------------------------------------
   -- Core clock
   ---------------------------------------------------------------------------- */

extern char __heapl2ram_start;
extern char __heapl2ram_size;

uint32_t system_core_clock = DEFAULT_SYSTEM_CLOCK;

/* ----------------------------------------------------------------------------
   -- SystemInit()
   ---------------------------------------------------------------------------- */
/* handler wrapper  */
//Handler_Wrapper_Light(fc_event_handler);

void system_core_clock_update(void)
{
    system_core_clock = pi_fll_get_frequency(FLL_SOC);
}

void system_init(void)
{
    /* Deactivate all soc events as they are active by default */
    SOCEU->FC_MASK_MSB = 0xFFFFFFFF;
    SOCEU->FC_MASK_LSB = 0xFFFFFFFF;

    /* FC Icache Enable*/
    SCBC->ICACHE_ENABLE = 0xFFFFFFFF;

    /* prepare to catch soc events, activate irqs 
     * then pmu init and core clock update 
     * (need to be in that order)
     */
    pi_fc_event_handler_init(FC_SOC_EVENT_IRQN);
    __enable_irq();

    pi_pmu_init();
    system_core_clock_update();
    /* Initialize our l2 tcdm malloc functions */
    pmsis_l2_malloc_init((void*)&__heapl2ram_start,(uint32_t)&__heapl2ram_size);
}

// Use timer low (32 bits), leaves one timer open
void system_setup_systick(uint32_t tick_rate_hz)
{
    /* Systick timer configuration. */
    SysTick->CFG_REG_LO = ( ( 1 << SysTick_CFG_REG_LOW_ENABLE_Pos )
            | ( 1 << SysTick_CFG_REG_LOW_RESET_Pos )
            | ( 1 << SysTick_CFG_REG_LOW_IRQE_Pos )
            | ( 0 << SysTick_CFG_REG_LOW_IEM_Pos )
            | ( 1 << SysTick_CFG_REG_LOW_CMP_CLR_Pos )
            | ( 0 << SysTick_CFG_REG_LOW_ONE_SHOT_Pos )
            | ( 0 << SysTick_CFG_REG_LOW_PRESCALERE_Pos )
            | ( 0 << SysTick_CFG_REG_LOW_CLKS_Pos )
            | ( 0 << SysTick_CFG_REG_LOW_PRESCALER_Pos )
            | ( 0 << SysTick_CFG_REG_LOW_64BIT_Pos )
            );
    /* Start the timer by putting a CMP value. */
    SysTick->CMP_LO = ( system_core_clock_get() / tick_rate_hz ) - 1ul;
    SysTick->VALUE_LO = 0;
    /* Enable IRQ from Systick timer. */
    NVIC->MASK_IRQ_OR = (0x1 << SYSTICK_IRQN);
}

uint32_t system_core_clock_get(void)
{
    /* might as well get latest frequency (fll stabilization etc) */
    system_core_clock_update();
    return system_core_clock;
}

/** Mostly useful for non regression testing **/
 __attribute__ ((noreturn)) void platform_exit(int code)
{
    if (pi_is_fc())
    {
        /* Write return value to APB device or do semihost exit */
        soc_ctrl_corestatus_set(code);
        if(code == 0)
        {
            gap8_semihost_exit(SEMIHOST_EXIT_SUCCESS);
        }
        else
        {
            gap8_semihost_exit(SEMIHOST_EXIT_ERROR);
        }
    }

    /* In case the platform does not support exit or this core is not allowed to exit the platform ... */
    hal_eu_evt_mask_clr(0xffffffff);
    hal_eu_evt_wait();

    while(1);
}
