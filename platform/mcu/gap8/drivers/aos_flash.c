#include "pmsis.h"
#include "board_flash.h"
#include "aos_flash.h"
#include "bsp/flash.h"
#include "aos/hal/flash.h"

extern aos_logical_partition_t hal_partitions[HAL_PARTITION_MAX];

#ifdef __cplusplus
extern "C" {
#endif
#if ( FLASH_BUS == FLASH_BUS_HYPERBUS )
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


#else
#if (FLASH_BUS == FLASH_BUS_QSPI)
pi_device_t pi_aos_flash_dev;
struct pi_spi_conf pi_aos_flash_conf;
//#define FLASH_CHIP_ERASE
#define QSPI_FLASH_CS 0

#define QSPIF_DUMMY_CYCLES(x)   ((uint8_t)((x&0xF)<<3))
#define QSPIF_BURST_LEN(x)      ((uint8_t)((x&0x3)<<0))
#define QSPIF_BURST_ENA(x)      ((uint8_t)((x&0x1)<<1))

#define QSPIF_WR_READ_REG_CMD   ((uint8_t)0x63)
#define QSPIF_WR_EN_CMD         ((uint8_t)0x06)
#define QSPIF_QPI_EN_CMD        ((uint8_t)0x35)
#define QSPIF_QPI_FAST_READ_CMD ((uint8_t)0x0b) 
#define QSPIF_PAGE_PROGRAM_CMD  ((uint8_t)0x02)
#define QSPIF_ERASE_SECTOR_CMD  ((uint8_t)0xD7)
#define QSPIF_READ_STATUS_CMD   ((uint8_t)0x05)

#define QSPIF_WR_PROLOGUE_SIZE 0x4
#define QSPIF_RCV_UCODE_SIZE   (28)
#define QSPIF_ERASE_SIZE       0x4
#define QSPIF_PAGE_SIZE        0x100

#define DUMMY_CYCLES 8

// ------ globals for some regular operations
static const uint8_t g_set_qspif_dummy[] = {QSPIF_WR_READ_REG_CMD,
    QSPIF_DUMMY_CYCLES(DUMMY_CYCLES)};
static const uint8_t g_enter_qpi_mode[] = {0x35,0};
static const uint8_t g_exit_qpi_mode[]  = {0xf5,0};
static const uint8_t g_chip_erase[]     = {0x60,0};
static const uint8_t g_write_enable[]   = {QSPIF_WR_EN_CMD,0};

PI_L2 static uint8_t g_qspif_status[4] = {0};
static inline void qpi_flash_enter_qpi(pi_device_t *qspi_dev)
{
    // Enter QPI mode
    pi_spi_send(qspi_dev, (void*)g_enter_qpi_mode, 1*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    // flash chip erase (optional!)
#ifdef FLASH_CHIP_ERASE
    pi_spi_send(qspi_dev, (void*)g_write_enable, 8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
    pi_spi_send(qspi_dev, (void*)g_chip_erase, 8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
    wait_wip(qspi_dev);
    printf("erase done\n");
    //pmsis_exit(0);
#endif
    // Set read parameters (dummy cycles)
    pi_spi_send(qspi_dev, (void*)g_write_enable, 8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
    pi_spi_send(qspi_dev, (void*)g_set_qspif_dummy, 2*8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
}

static inline void pi_qpi_flash_conf_spi(struct pi_spi_conf *conf)
{
    memset(conf, 0, sizeof(struct pi_spi_conf));
    pi_spi_conf_init(conf);
    conf->max_baudrate = 45000000;
    conf->wordsize = PI_SPI_WORDSIZE_8;
    conf->polarity = 0;
    conf->phase = conf->polarity;
    conf->cs = 0;
    conf->itf = 0;
    conf->big_endian = 1;
}

static inline void flash_conf_hook(pi_device_t *dev, struct pi_spi_conf *conf)
{
    memset(dev, 0, sizeof(pi_device_t));
    pi_qpi_flash_conf_spi(conf);
    pi_open_from_conf(dev,conf);
    pi_spi_open(dev);
    qpi_flash_enter_qpi(dev);
}


void wait_wip(pi_device_t *flash_dev)
{
    volatile uint8_t wip = 0;
    do
    {
        aos_msleep(1);
        g_qspif_status[0] = QSPIF_READ_STATUS_CMD; // read status reg
        pi_spi_send(flash_dev, (void*)&g_qspif_status[0], 1*8, PI_SPI_LINES_QUAD | PI_SPI_CS_KEEP);
        pi_spi_receive(flash_dev, (void*)&g_qspif_status[0], 1*8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
        wip = g_qspif_status[0] & 0x1;
    }while(wip);
}

void aos_flash_init(void)
{
    memset(&pi_aos_flash_dev, 0, sizeof(pi_device_t));
    flash_conf_hook(&pi_aos_flash_dev, &pi_aos_flash_conf);
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

    uint8_t *cmd_buf = pi_l2_malloc(QSPIF_ERASE_SIZE);
    for(uint32_t c_size = 0; c_size < size; c_size += sector_size)
    {
        uint32_t curr_flash_addr = flash_addr+c_size;
        cmd_buf[0] = QSPIF_ERASE_SECTOR_CMD;
        cmd_buf[1] = (curr_flash_addr & 0x00FF0000)>>16;
        cmd_buf[2] = (curr_flash_addr & 0x0000FF00)>>8;
        cmd_buf[3] = (curr_flash_addr & 0x000000FF)>>0;
        pi_spi_send(flash_dev, (void*)g_write_enable, 8,
                PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
        pi_spi_send(flash_dev, (void*)cmd_buf, 8*QSPIF_ERASE_SIZE,
                PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
        // wait typical erase time before looping
        wait_wip(flash_dev);
    }
    pi_l2_free(cmd_buf, QSPIF_ERASE_SIZE);
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

    uint8_t *l2_buff = pi_l2_malloc(QSPIF_PAGE_SIZE+QSPIF_WR_PROLOGUE_SIZE);
    if(!l2_buff)
    {
        printf("Malloc failed!\n");
        return -1;
    }
    uint32_t size_left = in_buf_len;
    uint32_t curr_size = 0;
    uint32_t curr_pos = 0;
    
    if((flash_addr & 0xFF) && (((flash_addr & 0xFF)+size_left) > 0x100))
    {
        curr_pos  = 0;
        curr_size = 0x100 - (flash_addr & 0xFF);
        size_left -= curr_size;
        memcpy(l2_buff+4, in_buf+curr_pos, curr_size);
        l2_buff[0] = QSPIF_PAGE_PROGRAM_CMD;
        l2_buff[1] = ((flash_addr+curr_pos) & 0x00FF0000)>>16;
        l2_buff[2] = ((flash_addr+curr_pos) & 0x0000FF00)>>8;
        l2_buff[3] = ((flash_addr+curr_pos) & 0x000000FF)>>0;
        // any write/erase op must be preceeded by a WRITE ENABLE op, with full CS cycling
        pi_spi_send(flash_dev, (void*)g_write_enable, 8,
                PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
        pi_spi_send(flash_dev, (void*)l2_buff, (curr_size+QSPIF_WR_PROLOGUE_SIZE)*8,
                PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
        wait_wip(flash_dev);
        curr_pos += curr_size;
    }

    while(size_left)
    {
        if(size_left >= QSPIF_PAGE_SIZE)
        {
            curr_size = QSPIF_PAGE_SIZE;
            size_left -= QSPIF_PAGE_SIZE;
        }
        else
        {
            curr_size = size_left;
            size_left = 0;
        }
        memcpy(l2_buff+4, in_buf+curr_pos, curr_size);
        l2_buff[0] = QSPIF_PAGE_PROGRAM_CMD;
        l2_buff[1] = ((flash_addr+curr_pos) & 0x00FF0000)>>16;
        l2_buff[2] = ((flash_addr+curr_pos) & 0x0000FF00)>>8;
        l2_buff[3] = ((flash_addr+curr_pos) & 0x000000FF)>>0;
        // any write/erase op must be preceeded by a WRITE ENABLE op, with full CS cycling
        pi_spi_send(flash_dev, (void*)g_write_enable, 8,
                PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
        pi_spi_send(flash_dev, (void*)l2_buff, (curr_size+QSPIF_WR_PROLOGUE_SIZE)*8,
                PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
        wait_wip(flash_dev);
        curr_pos += curr_size;
    }
    pi_l2_free(l2_buff, QSPIF_PAGE_SIZE+QSPIF_WR_PROLOGUE_SIZE);
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
    hal_flash_write(in_partition, off_set, in_buf, in_buf_len);
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

    uint8_t *l2_buff = pi_l2_malloc(QSPIF_PAGE_SIZE);
    uint8_t *ucode = pi_l2_malloc(QSPIF_RCV_UCODE_SIZE);
    uint32_t *ucode_u32 = (uint32_t*)ucode;
    if(!l2_buff)
    {
        printf("MALLOC FAILED\n");
        return -1;
    }
    uint32_t size_left = in_buf_len;
    uint32_t curr_size = 0;
    uint32_t curr_pos = 0;
    while(size_left)
    {
        if(size_left >= QSPIF_PAGE_SIZE)
        {
            curr_size = QSPIF_PAGE_SIZE;
            size_left -= QSPIF_PAGE_SIZE;
        }
        else
        {
            curr_size = size_left;
            size_left = 0;
        }
        ucode_u32[0] = pi_spi_get_config(flash_dev);
        ucode_u32[1] = SPI_CMD_SOT(0);
        ucode_u32[2] = SPI_CMD_TX_DATA(8*4, 1, 0);
        ucode[12] = QSPIF_QPI_FAST_READ_CMD;
        ucode[13] = ((flash_addr+curr_pos) & 0x00FF0000)>>16;
        ucode[14] = ((flash_addr+curr_pos) & 0x0000FF00)>>8;
        ucode[15] = ((flash_addr+curr_pos) & 0x000000FF)>>0;
        ucode_u32[4] = SPI_CMD_DUMMY(DUMMY_CYCLES);
        ucode_u32[5] = SPI_CMD_RX_DATA(curr_size*8, 1, 0);
        ucode_u32[6] = SPI_CMD_EOT(1);
        // any write/erase op must be preceeded by a WRITE ENABLE op, with full CS cycling
        pi_spi_receive_with_ucode(flash_dev, (void*)l2_buff, (curr_size)*8,
                PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO, 1, QSPIF_RCV_UCODE_SIZE, ucode );
        memcpy(out_buf+curr_pos, l2_buff, curr_size);
        curr_pos += curr_size;
    }
    pi_l2_free(l2_buff, QSPIF_PAGE_SIZE);
    pi_l2_free(ucode,QSPIF_RCV_UCODE_SIZE);
    *off_set += in_buf_len;
    return 0;
}

#endif
#endif

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

        if((addr >= part_min) && (addr <= part_max))
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
