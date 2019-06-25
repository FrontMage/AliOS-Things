/*
 * Copyright 2017-2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include "pmsis.h"
#include "bsp/flash.h"
#include "board_flash.h"
#include "aos_flash.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern pi_device_t pi_aos_flash_dev;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Initialize debug console. */
void board_init_debug_console(void)
{
    //CLOCK_SetIpSrc(kCLOCK_Lpuart0, kCLOCK_IpSrcFircAsync);
}

void board_init(void)
{
#ifdef GAPUINO8_HYPERFLASH_SUPPORT
    aos_flash_init();
    gapuino8_flash_partition_init(&pi_aos_flash_dev);
#endif
}
