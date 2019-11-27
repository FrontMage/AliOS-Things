#include "aos/hal/flash.h"
#include "board_flash.h"
#include "aos_flash.h"
#include "pmsis.h"

/* Logic partition on flash devices */
aos_logical_partition_t hal_partitions[HAL_PARTITION_MAX];

void gapuino8_flash_partition_init(pi_device_t *flash_dev)
{
    for(int i = 0; i < HAL_PARTITION_MAX; i++)
    {
        memset(&hal_partitions[i], 0, sizeof(aos_logical_partition_t));
    }
    hal_partitions[HAL_PARTITION_BOOTLOADER].aos_partition.partition_owner       = HAL_FLASH_QSPI;
    hal_partitions[HAL_PARTITION_BOOTLOADER].aos_partition.partition_description = "bootloader";
    hal_partitions[HAL_PARTITION_BOOTLOADER].aos_partition.partition_start_addr  = 0x00000000;
    hal_partitions[HAL_PARTITION_BOOTLOADER].aos_partition.partition_length      = 0x80000; /* 512k bytes -- 4KB sectors*/
    hal_partitions[HAL_PARTITION_BOOTLOADER].aos_partition.partition_options     = PAR_OPT_READ_EN;
    hal_partitions[HAL_PARTITION_BOOTLOADER].flash_dev                           = flash_dev;
    hal_partitions[HAL_PARTITION_BOOTLOADER].sector_size                         = 0x1000;

    hal_partitions[HAL_PARTITION_PARAMETER_2].aos_partition.partition_owner       = HAL_FLASH_QSPI;
    hal_partitions[HAL_PARTITION_PARAMETER_2].aos_partition.partition_description = "KV object storage";
    hal_partitions[HAL_PARTITION_PARAMETER_2].aos_partition.partition_start_addr  = 0x00080000;
    hal_partitions[HAL_PARTITION_PARAMETER_2].aos_partition.partition_length      = 0x8000; /* 32k bytes -- 4KB sectors */
    hal_partitions[HAL_PARTITION_PARAMETER_2].aos_partition.partition_options     = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN;
    hal_partitions[HAL_PARTITION_PARAMETER_2].flash_dev                           = flash_dev;
    hal_partitions[HAL_PARTITION_PARAMETER_2].sector_size                         = 0x1000;

    hal_partitions[HAL_PARTITION_SPIFFS].aos_partition.partition_owner          = HAL_FLASH_QSPI;
    hal_partitions[HAL_PARTITION_SPIFFS].aos_partition.partition_description    = "Spiffs data";
    hal_partitions[HAL_PARTITION_SPIFFS].aos_partition.partition_start_addr     = 0x00088000;
    hal_partitions[HAL_PARTITION_SPIFFS].aos_partition.partition_length         = 0x00F00000; /* 15M bytes -- 4KB sectors*/
    hal_partitions[HAL_PARTITION_SPIFFS].aos_partition.partition_options        = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN;
    hal_partitions[HAL_PARTITION_SPIFFS].flash_dev                              = flash_dev;
    hal_partitions[HAL_PARTITION_SPIFFS].sector_size                            = 0x1000;
}
