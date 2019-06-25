/*
 *  some type definition for help with aos uart 
 *
 */

#ifndef __GAP8_AOS_FLASH_H__
#define __GAP8_AOS_FLASH_H__
#include "aos/hal/flash.h"
typedef struct {
    hal_logic_partition_t aos_partition;
    pi_device_t *flash_dev;
    uint32_t sector_size;
} aos_logical_partition_t;

void aos_flash_init(void);

#endif
