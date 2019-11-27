#ifndef __GAPUINO8_BOARD_FLASH_H__
#define __GAPUINO8_BOARD_FLASH_H__

#include "pmsis.h"
#include "aos_flash.h"

#define FLASH_BUS FLASH_BUS_HYPERBUS

void gapuino8_flash_partition_init(pi_device_t *flash_dev);
#endif
