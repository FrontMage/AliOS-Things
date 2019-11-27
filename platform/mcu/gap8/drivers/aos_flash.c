#include "pmsis.h"
#include "board_flash.h"
#include "aos_flash.h"
#include "bsp/flash.h"
#include "aos/hal/flash.h"
#if ( FLASH_BUS == FLASH_BUS_HYBERBUS )
#include "bsp/flash/hyperflash.h"

#define HYPERFLASH_SET_PARAMETER_HIGH ((1<<8)&0x3)
#define HYPERFLASH_SET_PARAMETER_LOW ((0<<8)&0x3)

static void aos_hyperflash_set_parameter_area(pi_device_t *dev, uint32_t param);

#define flash_conf_hook(dev,conf)\
{\
    pi_hyperflash_conf_init(conf);\
    pi_open_from_conf(dev,conf);\
    pi_flash_open(dev);\
    aos_hyperflash_set_parameter_area(dev, HYPERFLASH_SET_PARAMETER_HIGH);\
    printf("flash set param area done\n");\
}

pi_device_t pi_aos_flash_dev;
struct pi_hyperflash_conf pi_aos_flash_conf;

#else
#if (FLASH_BUS == FLASH_BUS_QSPI)
#warning "not yet supported"
#endif


#ifdef __cplusplus
extern "C" {
#endif
#endif
extern aos_logical_partition_t hal_partitions[HAL_PARTITION_MAX];

void aos_flash_init(void)
{
    memset(&pi_aos_flash_dev, 0, sizeof(pi_device_t));
    memset(&pi_aos_flash_conf, 0, sizeof(struct pi_hyperflash_conf));
    flash_conf_hook(&pi_aos_flash_dev, &pi_aos_flash_conf);
}

static void aos_hyperflash_set_parameter_area(pi_device_t *dev, uint32_t param)
{

    printf("going to play with registers\n");
    // ------ setup for read Non volatile config value   
    uint32_t value  = 0xAA;
    pi_flash_reg_set(dev, 0x555<<1, (uint8_t*)&value);
    value  = 0x55;
    pi_flash_reg_set(dev, 0x2AA<<1, (uint8_t*)&value);
    value  = 0xC6;
    pi_flash_reg_set(dev, 0x555<<1, (uint8_t*)&value);
    // ------ get the value
    pi_flash_reg_get(dev, 0, (uint8_t*)&value);

    value &= ~param;
    value |= param;
    uint32_t vcr = value;

    value  = 0xAA;
    pi_flash_reg_set(dev, 0x555<<1, (uint8_t*)&value);
    value  = 0x55;
    pi_flash_reg_set(dev, 0x2AA<<1, (uint8_t*)&value);
    value  = 0x38;
    pi_flash_reg_set(dev, 0x555<<1, (uint8_t*)&value);
    // ------ get the value
    pi_flash_reg_set(dev, 0, (uint8_t*)&vcr);
}

/**
 * Get the information of the specified flash area
 *
 * @param[in]  in_partition     The target flash logical partition
 * @param[in]  partition        The buffer to store partition info
 *
 * @return  0: On success， otherwise is error
 */
int32_t hal_flash_info_get(hal_partition_t in_partition, hal_logic_partition_t *partition)
{
    *partition = hal_partitions[in_partition].aos_partition;
    return 0;
}

/**
 * Erase an area on a Flash logical partition
 *
 * @note  Erase on an address will erase all data on a sector that the
 *        address is belonged to, this function does not save data that
 *        beyond the address area but in the affected sector, the data
 *        will be lost.
 *
 * @param[in]  in_partition  The target flash logical partition which should be erased
 * @param[in]  off_set       Start address of the erased flash area
 * @param[in]  size          Size of the erased flash area
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t hal_flash_erase(hal_partition_t in_partition, uint32_t off_set, uint32_t size)
{
    uint32_t flash_addr = hal_partitions[in_partition].aos_partition.partition_start_addr + off_set;
    pi_device_t *flash_dev  = hal_partitions[in_partition].flash_dev;
    uint32_t sector_size = hal_partitions[in_partition].sector_size;

    for(uint32_t c_size = 0; c_size < size; c_size += sector_size)
    {
#if 0
        pi_flash_reg_set(flash_dev, 0x555<<1, 0xAA);
        pi_flash_reg_set(flash_dev, 0x2AA<<1, 0x55);
        pi_flash_reg_set(flash_dev, 0x555<<1, 0x80);
        pi_flash_reg_set(flash_dev, 0x555<<1, 0xAA);
        pi_flash_reg_set(flash_dev, 0x2AA<<1, 0x55);
        pi_flash_reg_set(flash_dev, flash_addr+c_size, 0x30);
#else
        pi_flash_erase_sector(flash_dev, flash_addr+c_size);
#endif
    }
    return 0;
}

/**
 * Write data to an area on a flash logical partition without erase
 *
 * @param[in]  in_partition    The target flash logical partition which should be read which should be written
 * @param[in]  off_set         Point to the start address that the data is written to, and
 *                             point to the last unwritten address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  inBuffer        point to the data buffer that will be written to flash
 * @param[in]  inBufferLength  The length of the buffer
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t hal_flash_write(hal_partition_t in_partition, uint32_t *off_set,
                        const void *in_buf, uint32_t in_buf_len)
{
    uint32_t flash_addr = hal_partitions[in_partition].aos_partition.partition_start_addr + *off_set;
    pi_device_t *flash_dev  = hal_partitions[in_partition].flash_dev;

    if(((uintptr_t)in_buf & 0xFFF00000) != 0x1C000000)
    {
        void *l2_buff = pi_l2_malloc(in_buf_len);
        if(!l2_buff)
        {
            return -1;
        }
        memcpy(l2_buff, in_buf, in_buf_len);
        pi_flash_program(flash_dev, flash_addr, l2_buff, in_buf_len);
        pi_l2_free(l2_buff, in_buf_len);
    }
    else
    {
        pi_flash_program(flash_dev, flash_addr, (void*)in_buf, in_buf_len);
    }
    *off_set += in_buf_len;
    return 0;
}

/**
 * Write data to an area on a flash logical partition with erase first
 *
 * @param[in]  in_partition    The target flash logical partition which should be read which should be written
 * @param[in]  off_set         Point to the start address that the data is written to, and
 *                             point to the last unwritten address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  inBuffer        point to the data buffer that will be written to flash
 * @param[in]  inBufferLength  The length of the buffer
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t hal_flash_erase_write(hal_partition_t in_partition, uint32_t *off_set,
                              const void *in_buf, uint32_t in_buf_len)
{
    uint32_t flash_addr = hal_partitions[in_partition].aos_partition.partition_start_addr + *off_set;
    pi_device_t *flash_dev  = hal_partitions[in_partition].flash_dev;

    hal_flash_erase(in_partition, *off_set, in_buf_len);
    if(((uintptr_t)in_buf & 0xFFF00000) != 0x1C000000)
    {
        void *l2_buff = pi_l2_malloc(in_buf_len);
        if(!l2_buff)
        {
            return -1;
        }
        memcpy(l2_buff, in_buf, in_buf_len);
        pi_flash_program(flash_dev, flash_addr, l2_buff, in_buf_len);
        pi_l2_free(l2_buff, in_buf_len);
    }
    else
    {
        pi_flash_program(flash_dev, flash_addr, (void*)in_buf, in_buf_len);
    }
    *off_set += in_buf_len;
    return 0;
}

/**
 * Read data from an area on a Flash to data buffer in RAM
 *
 * @param[in]  in_partition    The target flash logical partition which should be read
 * @param[in]  off_set         Point to the start address that the data is read, and
 *                             point to the last unread address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  outBuffer       Point to the data buffer that stores the data read from flash
 * @param[in]  inBufferLength  The length of the buffer
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t hal_flash_read(hal_partition_t in_partition, uint32_t *off_set,
                       void *out_buf, uint32_t in_buf_len)
{
    uint32_t flash_addr = hal_partitions[in_partition].aos_partition.partition_start_addr + *off_set;
    pi_device_t *flash_dev  = hal_partitions[in_partition].flash_dev;

    if(((uintptr_t)out_buf & 0xFFF00000) != 0x1C000000)
    {
        void *l2_buff = pi_l2_malloc(in_buf_len);
        if(!l2_buff)
        {
            return -1;
        }
        pi_flash_read(flash_dev, flash_addr, l2_buff, in_buf_len);
        memcpy(out_buf, l2_buff, in_buf_len);
        pi_l2_free(l2_buff, in_buf_len);
    }
    else
    {
        pi_flash_read(flash_dev, flash_addr, out_buf, in_buf_len);
    }
    *off_set += in_buf_len;
    return 0;
}

/**
 * Set security options on a logical partition
 *
 * @param[in]  partition  The target flash logical partition
 * @param[in]  offset     Point to the start address that the data is read, and
 *                        point to the last unread address after this function is
 *                        returned, so you can call this function serval times without
 *                        update this start address.
 * @param[in]  size       Size of enabled flash area
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t hal_flash_enable_secure(hal_partition_t partition, uint32_t off_set, uint32_t size)
{
    return -1;
}

/**
 * Disable security options on a logical partition
 *
 * @param[in]  partition  The target flash logical partition
 * @param[in]  offset     Point to the start address that the data is read, and
 *                        point to the last unread address after this function is
 *                        returned, so you can call this function serval times without
 *                        update this start address.
 * @param[in]  size       Size of disabled flash area
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t hal_flash_dis_secure(hal_partition_t partition, uint32_t off_set, uint32_t size)
{
    return -1;
}

/**
 * Convert physical address to logic partition id and offset in partition
 *
 * @param[out]  in_partition Point to the logic partition id
 * @param[out]  off_set      Point to the offset in logic partition
 * @param[in]   addr         The physical address
 *
 * @return 0 : On success, EIO : If an error occurred with any step
 */
int32_t hal_flash_addr2offset(hal_partition_t *in_partition, uint32_t *off_set, uint32_t addr)
{
     for(int i = 0; i < HAL_PARTITION_MAX; i++)
     {
        uint32_t part_min = hal_partitions[i].aos_partition.partition_start_addr;
        uint32_t part_max = hal_partitions[i].aos_partition.partition_start_addr
            + (hal_partitions[i].aos_partition.partition_length-1);

        if((addr > part_min) && (addr < part_max))
        {
            *in_partition = (hal_partition_t) i;
            *off_set =  addr-part_min;
            return 0;
        }
     }
     return -1;
}

#ifdef __cplusplus
}
#endif
