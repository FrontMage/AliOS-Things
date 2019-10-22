/*
 * Copyright 2019 Greenwaves Technologies
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include "pmsis.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Initialize debug console. */
void board_init_debug_console(void)
{
#if ((defined(PRINTF_USE_UART) || defined(GAPOC_USE_UART))\
        && !defined(GAPOC_NO_UART))
    // Init B2 gpio pad
    // enable pull (down/up)
    hal_gpio_pin_enable_pull(1, 1);
    pi_pad_set_function(PI_PAD_13_B2_RF_PACTRL1, PI_PAD_9_B3_GPIO_A1_FUNC1);
    
    // set as High Z: disable pull, set as input, no clock
    hal_gpio_pin_set_direction(1, 0x0);
    hal_gpio_pin_enable_pull(1, 0);
    hal_gpio_pin_enable(1, 0x0);
    //------ B2 done


    // ------ Init B2 pad with A1 gpio & value 1 to trigg UART MUX
    // set pin B2 to 1
    // set B2 to NOPULL
    hal_gpio_pin_enable_pull(1,0);
    // Set B2 to A1 GPIO
    pi_pad_set_function(PI_PAD_13_B2_RF_PACTRL1, PI_PAD_13_B2_GPIO_A1_FUNC1);
    // cut the clock (can't read back, save energy)
    hal_gpio_pin_enable(1, 0x0);
    // write output value
    hal_gpio_pin_set_output_value(1, 1);
    // output mode
    hal_gpio_pin_set_direction(1, 0x1);

    
    // Only works without Lora module: set pin rx to not point toware ble
    // TODO: use aos_ API instead of ll hal
    hal_gpio_pin_enable_pull(7,0);
    hal_gpio_pin_enable_pull(24,0);
#endif
}
