#ifndef __GAPUINO8_BOARD_FLASH_H__
#define __GAPUINO8_BOARD_FLASH_H__

#include "pmsis.h"
#include "aos_flash.h"

#define FLASH_BUS_HYPERBUS 0
#define FLASH_BUS_QSPI 1
#define FLASH_BUS FLASH_BUS_QSPI

void gap8_breakout_flash_partition_init(pi_device_t *flash_dev);
#endif
