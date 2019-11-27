/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>

#include "pmsis.h"
#include "gap_semihost.h"
#include <aos/kernel.h>

#if 0
void __pi_spi_receive_async_with_ucode(struct spim_cs_data *cs_data, void *data, size_t len,
        pi_spi_flags_e flags, int use_ucode, int ucode_size,
        void *ucode, pi_task_t *task)
{
    struct spim_driver_data *drv_data = SPIM_CS_DATA_GET_DRV_DATA(cs_data);
    int qspi    = ((flags >> 2) & 0x3) == PI_SPI_LINES_QUAD;
    int cs_mode = (flags >> 0) & 0x3;

    int device_id = drv_data->device_id;
    DBG_PRINTF("%s:%d: core clock:%d, baudrate:%d, div=%d, udma_cmd cfg =%lx\n",
            __func__,__LINE__,system_core_clock_get(),cs_data->max_baudrate,
            system_core_clock_get() / cs_data->max_baudrate,cfg);
    uint32_t byte_align = (cs_data->wordsize == PI_SPI_WORDSIZE_32)
        && cs_data->big_endian;
    int size = (len + 7) >> 3;

    int cmd_id = 0;

    int irq = __disable_irq();
    if(!drv_data->end_of_transfer)
    {
        if(use_ucode)
        {
            //cs_data->udma_cmd[cmd_id++] = SPI_CMD_TX_DATA(8*4, qspi, byte_align);
            printf("insert ucode\n");
            //memcpy(&(cs_data->udma_cmd[cmd_id]), ucode, ucode_size);
            //cmd_id += (ucode_size/4);
        }
        drv_data->end_of_transfer = task;
        drv_data->repeat_transfer = NULL;
        if(cs_mode == PI_SPI_CS_AUTO)
        {
            //cs_data->udma_cmd[cmd_id++] = SPI_CMD_EOT(1);
        }
        else
        {
            hal_soc_eu_set_fc_mask(SOC_EVENT_UDMA_SPIM_RX(device_id));
        }

        for(int i = 0; i<ucode_size; i++)
        {
            printf("ucode[i]=%x\n",((uint8_t*)ucode)[i]);
        }
        spim_enqueue_channel(SPIM(device_id), (uint32_t)data, size,
                UDMA_CORE_RX_CFG_EN(1) | (2<<1), RX_CHANNEL);
        spim_enqueue_channel(SPIM(device_id), (uint32_t)ucode,
                ucode_size, UDMA_CORE_TX_CFG_EN(1), TX_CHANNEL);
    }
    else
    {
#if 0
        struct spim_transfer transfer;
        transfer.data = data;
        transfer.flags = flags;
        transfer.len = len;
        transfer.cfg_cmd = cfg;
        transfer.byte_align = byte_align;
        transfer.is_send = 0;
        __pi_spim_drv_fifo_enqueue(cs_data, &transfer, task);
#endif
    }
    restore_irq(irq);
}

#endif


PI_L2 char write_string[] = {'h','e','l','l','o',' ','f','r','i','e','n','d','s','\n','\0'};

PI_L2 volatile uint8_t data[256+5];
PI_L2 volatile uint8_t data_r[256] = {0};

PI_L2 pi_device_t qspi_dev0 = {0};

PI_L2 uint32_t g_zero = 0;

#define QSPI_FLASH_CS 0

#define QSPIF_DUMMY_CYCLES(x)   ((uint8_t)((x&0xF)<<3))
#define QSPIF_BURST_LEN(x)      ((uint8_t)((x&0x3)<<0))
#define QSPIF_BURST_ENA(x)      ((uint8_t)((x&0x1)<<1))

#define QSPIF_WR_READ_REG_CMD   ((uint8_t)0x63)
#define QSPIF_WR_EN_CMD         ((uint8_t)0x06)
#define QSPIF_QPI_EN_CMD        ((uint8_t)0x35)
#define QSPIF_QPI_FAST_READ     ((uint8_t)0x0b) 

#define DUMMY_CYCLES 8

extern void pi_spi_receive_with_ucode(struct pi_device *device, void *data,
        size_t len, pi_spi_flags_e flags, int use_ucode, int ucode_size,
        void *ucode);
extern uint32_t pi_spi_get_config(struct pi_device *device);

//uint8_t g_set_qspif_dummy[] = {QSPIF_WR_EN_CMD, QSPIF_WR_READ_REG_CMD,
//    QSPIF_DUMMY_CYCLES(DUMMY_CYCLES) | QSPIF_BURST_LEN(3) | QSPIF_BURST_ENA(1)};
// set 2 dummy cycles @ 50MHz
uint8_t g_set_qspif_dummy[] = {QSPIF_WR_READ_REG_CMD,
    QSPIF_DUMMY_CYCLES(DUMMY_CYCLES)};
uint8_t g_enter_qpi_mode[] = {0x35,0,0,0};
uint8_t g_exit_qpi_mode[]  = {0xf5,0,0,0};

uint8_t g_erase_block_0[] = {0xD7, 0, 0x20, 0, 0, 0, 0,0};
uint8_t g_buf[32] = {0};

int single_lanes_test()
{
    int count = 0;
    int errors = 0;
    printf("Entering single lane spi flash test!\n");

    aos_msleep(1000);
    // Add wr enable, then send cmd, addr, and finally, data from index 5
    //data[0] = QSPIF_WR_EN_CMD;
    data[0] = 0x02;
    data[1] = 0x0;
    data[2] = 0x10;
    data[3] = 0x0;

    for(int i = 0; i<256; i++)
    {
        data[i+4] = i+1;
    }

    struct pi_spi_conf spi_conf;
    memset(&spi_conf, 0, sizeof(struct pi_spi_conf));

    pi_spi_conf_init(&spi_conf);
    spi_conf.max_baudrate = system_core_clock_get(); // 50MHz for now
    spi_conf.wordsize = PI_SPI_WORDSIZE_8;
    spi_conf.polarity = 0;
    spi_conf.phase = spi_conf.polarity;
    spi_conf.cs = 0;
    spi_conf.itf = 0;
    spi_conf.big_endian = 1;

    pi_open_from_conf(&qspi_dev0, &spi_conf);
    int ret = pi_spi_open(&qspi_dev0);

    printf("ret=%x\n",ret);

    // init flash --> setup dummy cycles first
#if 0
    g_buf[0] = 0x66;
    pi_spi_transfer(&qspi_dev0, (void*)&g_buf[0], (void*)&g_buf[4],8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    aos_msleep(100);
    g_buf[0] = 0x99;
    pi_spi_transfer(&qspi_dev0, (void*)&g_buf[0], (void*)&g_buf[4],8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    aos_msleep(100);
    printf("------------Reset done-------------\n");
#endif
    
    //g_buf[0] = QSPIF_WR_EN_CMD;
    //pi_spi_transfer(&qspi_dev0, (void*)g_buf, (void*)&g_buf[4],8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);

    g_buf[0] = 0x9F;
    pi_spi_transfer(&qspi_dev0, (void*)&g_buf[0], (void*)&g_buf[4], 8, PI_SPI_LINES_SINGLE | PI_SPI_CS_KEEP);
    pi_spi_transfer(&qspi_dev0, (void*)&g_zero, (void*)&g_buf[0], 8*3, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    printf("jdec id reg= %x\n",g_buf[0]);
    printf("jdec id reg= %x\n",g_buf[1]);
    printf("jdec id reg= %x\n",g_buf[2]);

    // flash is now in QPI mode, with 2 dummy read cycles
    // -------------------------------------------------
    
    // Erase block 0
    
    g_buf[0] = QSPIF_WR_EN_CMD;
    pi_spi_transfer(&qspi_dev0, (void*)g_buf, (void*)&g_buf[4],8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);

    pi_spi_send(&qspi_dev0, (void*)g_erase_block_0, 4*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    uint8_t wip = 0;
    do
    {
        g_buf[0] = 0x05;
        pi_spi_send(&qspi_dev0, (void*)g_buf, 1*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_KEEP);
        pi_spi_receive(&qspi_dev0, (void*)g_buf, 1*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
        printf("g_buf[0]=%i\n",g_buf[0]);
        wip = g_buf[0] & 0x1;
    }while(wip);
    // Write
    g_buf[0] = QSPIF_WR_EN_CMD;
    pi_spi_transfer(&qspi_dev0, (void*)g_buf, (void*)&g_buf[4],8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    pi_spi_send(&qspi_dev0, (void*)data, 260*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO); 
    wip = 0;
    do
    {
        g_buf[0] = 0x05; // read status reg
        pi_spi_send(&qspi_dev0, (void*)g_buf, 1*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_KEEP);
        pi_spi_receive(&qspi_dev0, (void*)g_buf, 1*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
        printf("g_buf[0]=%i\n",g_buf[0]);
        wip = g_buf[0] & 0x1;
    }while(wip);

    // now, read back the data
    g_buf[0] = 0x3;
    g_buf[1] = 0;
    g_buf[2] = 0x10;
    g_buf[3] = 0;
    //*(uint32_t*)(&g_buf[4]) = SPI_CMD_DUMMY(DUMMY_CYCLES);

    printf("data_r: %x\n",data_r);
    pi_spi_send(&qspi_dev0, (void*)g_buf, 8*4, PI_SPI_LINES_SINGLE | PI_SPI_CS_KEEP);
    pi_spi_receive(&qspi_dev0, (void*)data_r, 256*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);

    for(int i = 0; i<256; i++)
    {
        if(data_r[i] != data[i+4])
        {
            printf("data_r[%i]=%x, data[%i+5]=%x\n",i,data_r[i],i,data[i+5]);
            errors ++;
        }
    }
    pi_spi_close(&qspi_dev0);
    printf("there was %d errors\n",errors);
    return errors;
}

int qpi_flash_test(void)
{
    int count = 0;
    int errors = 0;
    printf("nano entry here!\n");

    aos_msleep(1000);
    // Add wr enable, then send cmd, addr, and finally, data from index 5
    //data[0] = QSPIF_WR_EN_CMD;
    data[0] = 0x02;
    data[1] = 0x0;
    data[2] = 0x20;
    data[3] = 0x0;

    for(int i = 0; i<256; i++)
    {
        data[i+4] = i+1;
    }

    struct pi_spi_conf spi_conf;
    memset(&spi_conf, 0, sizeof(struct pi_spi_conf));

    pi_spi_conf_init(&spi_conf);
    spi_conf.max_baudrate = system_core_clock_get(); // 50MHz for now
    spi_conf.wordsize = PI_SPI_WORDSIZE_8;
    spi_conf.polarity = 0;
    spi_conf.phase = spi_conf.polarity;
    spi_conf.cs = 0;
    spi_conf.itf = 0;
    spi_conf.big_endian = 1;

    pi_open_from_conf(&qspi_dev0, &spi_conf);
    int ret = pi_spi_open(&qspi_dev0);

    printf("ret=%x\n",ret);

    // init flash --> setup dummy cycles first
#if 0
    g_buf[0] = 0x66;
    pi_spi_transfer(&qspi_dev0, (void*)&g_buf[0], (void*)&g_buf[4],8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    aos_msleep(100);
    g_buf[0] = 0x99;
    pi_spi_transfer(&qspi_dev0, (void*)&g_buf[0], (void*)&g_buf[4],8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    aos_msleep(100);
    printf("------------Reset done-------------\n");
#endif
    
    //g_buf[0] = QSPIF_WR_EN_CMD;
    //pi_spi_transfer(&qspi_dev0, (void*)g_buf, (void*)&g_buf[4],8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);

    g_buf[0] = 0x9F;
    pi_spi_send(&qspi_dev0, (void*)&g_buf[0], 8, PI_SPI_LINES_SINGLE | PI_SPI_CS_KEEP);
    pi_spi_receive(&qspi_dev0, (void*)&g_buf[0], 8*3, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    printf("jdec id reg= %x\n",g_buf[0]);
    printf("jdec id reg= %x\n",g_buf[1]);
    printf("jdec id reg= %x\n",g_buf[2]);

    g_buf[0] = QSPIF_WR_EN_CMD;
    pi_spi_send(&qspi_dev0, (void*)g_buf, 8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);

    pi_spi_send(&qspi_dev0, (void*)g_set_qspif_dummy, 2*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    g_buf[0] = 0x61;
    pi_spi_send(&qspi_dev0, (void*)g_buf, 1*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_KEEP);
    pi_spi_receive(&qspi_dev0, (void*)g_buf, 1*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    printf("read param reg= %x\n",g_buf[0]),
    // then QPI mode enter
    printf("g_enter_qpi_mode: %x\n",g_enter_qpi_mode);
    pi_spi_send(&qspi_dev0, (void*)g_enter_qpi_mode, 1*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    // flash is now in QPI mode, with 2 dummy read cycles
    // -------------------------------------------------
    
    // Erase block 0
    
    g_buf[0] = QSPIF_WR_EN_CMD;
    pi_spi_send(&qspi_dev0, (void*)g_buf,8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);

    pi_spi_send(&qspi_dev0, (void*)g_erase_block_0, 4*8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
    uint8_t wip = 0;
    do
    {
        g_buf[0] = 0x05;
        pi_spi_send(&qspi_dev0, (void*)g_buf, 1*8, PI_SPI_LINES_QUAD | PI_SPI_CS_KEEP);
        pi_spi_receive(&qspi_dev0, (void*)g_buf, 1*8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
        printf("g_buf[0]=%i\n",g_buf[0]);
        wip = g_buf[0] & 0x1;
    }while(wip);
    // Write
    g_buf[0] = QSPIF_WR_EN_CMD;
    pi_spi_send(&qspi_dev0, (void*)g_buf, 8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
    pi_spi_send(&qspi_dev0, (void*)data, 260*8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO); 
    wip = 0;
    do
    {
        g_buf[0] = 0x05; // read status reg
        pi_spi_send(&qspi_dev0, (void*)g_buf, 1*8, PI_SPI_LINES_QUAD | PI_SPI_CS_KEEP);
        pi_spi_receive(&qspi_dev0, (void*)g_buf, 1*8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
        printf("g_buf[0]=%i\n",g_buf[0]);
        wip = g_buf[0] & 0x1;
    }while(wip);

    uint32_t len = 256*8;
    volatile uint32_t *g_buf_u32 = (uint32_t*)g_buf;
    g_buf_u32[0] = pi_spi_get_config(&qspi_dev0);
    g_buf_u32[1] = SPI_CMD_SOT(0);
    g_buf_u32[2] = SPI_CMD_TX_DATA(8*4, 1, 0);
    g_buf[12] = 0xb;
    g_buf[13] = 0;
    g_buf[14] = 0x20;
    g_buf[15] = 0;
    g_buf_u32[4] = SPI_CMD_DUMMY(DUMMY_CYCLES);
    g_buf_u32[5] = SPI_CMD_RX_DATA(len, 1, 0);
    g_buf_u32[6] = SPI_CMD_EOT(1);

    printf("data_r: %x\n",data_r);
    printf("g_buf: %x\n",g_buf);
    //pi_spi_send(&qspi_dev0, (void*)g_buf, 8*8, PI_SPI_LINES_QUAD | PI_SPI_CS_KEEP);
    pi_spi_receive_with_ucode(&qspi_dev0, (void*)data_r, len,
            PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO, 1, 28, g_buf);

    for(int i = 0; i<256; i++)
    {
        if(data_r[i] != data[i+4])
        {
            printf("data_r[%i]=%x, data[%i+4]=%x\n",i,data_r[i],i,data[i+4]);
            errors ++;
        }
    }

    pi_spi_send(&qspi_dev0, (void*)g_exit_qpi_mode, 1*8, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
    printf("there was %d errors\n",errors);
    return errors;
}

int application_start(int argc, char *argv[])
{
    return qpi_flash_test();
}
