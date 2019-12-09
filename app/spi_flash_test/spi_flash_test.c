/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>

#include "pmsis.h"
#include "gap_semihost.h"
#include <aos/kernel.h>

// prepare L2 buffers to simplify
PI_L2 volatile uint8_t data[256+5];
PI_L2 volatile uint8_t data_r[256] = {0};

PI_L2 pi_device_t qspi_dev0 = {0};

#define QSPI_FLASH_CS 0

#define QSPIF_DUMMY_CYCLES(x)   ((uint8_t)((x&0xF)<<3))
#define QSPIF_BURST_LEN(x)      ((uint8_t)((x&0x3)<<0))
#define QSPIF_BURST_ENA(x)      ((uint8_t)((x&0x1)<<1))

#define QSPIF_WR_READ_REG_CMD   ((uint8_t)0x63)
#define QSPIF_WR_EN_CMD         ((uint8_t)0x06)
#define QSPIF_QPI_EN_CMD        ((uint8_t)0x35)
#define QSPIF_QPI_FAST_READ     ((uint8_t)0x0b) 

#define DUMMY_CYCLES 8

uint8_t g_set_qspif_dummy[] = {QSPIF_WR_READ_REG_CMD,
    QSPIF_DUMMY_CYCLES(DUMMY_CYCLES)};
uint8_t g_enter_qpi_mode[] = {0x35,0,0,0};
uint8_t g_exit_qpi_mode[]  = {0xf5,0,0,0};

uint8_t g_erase_block_0[] = {0xD7, 0, 0x10, 0, 0, 0, 0, 0};
uint8_t g_buf[32] = {0};

int single_lanes_test()
{
    int count = 0;
    int errors = 0;
    printf("Entering single lane spi flash test!\n");

    aos_msleep(1000);
    // Add wr enable, then send cmd, addr, and finally, data from index 5
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
    spi_conf.max_baudrate = 45000000; // 50MHz for now
    spi_conf.wordsize = PI_SPI_WORDSIZE_8;
    spi_conf.polarity = 0;
    spi_conf.phase = spi_conf.polarity;
    spi_conf.cs = 0;
    spi_conf.itf = 0;
    spi_conf.big_endian = 1;

    pi_open_from_conf(&qspi_dev0, &spi_conf);
    int ret = pi_spi_open(&qspi_dev0);

    printf("ret=%x\n",ret);

    g_buf[0] = 0x9F;
    pi_spi_send(&qspi_dev0, (void*)&g_buf[0], 8, PI_SPI_LINES_SINGLE | PI_SPI_CS_KEEP);
    pi_spi_receive(&qspi_dev0, (void*)&g_buf[0], 8*3, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    printf("jdec id reg = %x:%x:%x\n",g_buf[0],g_buf[1],g_buf[2]);
    
    // Erase block 0
    g_buf[0] = QSPIF_WR_EN_CMD;
    pi_spi_send(&qspi_dev0, (void*)g_buf,8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
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
    pi_spi_send(&qspi_dev0, (void*)g_buf, 8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
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
    printf("QSPI flash test\n");

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
    spi_conf.max_baudrate = 45000000; // 50MHz for now
    spi_conf.wordsize = PI_SPI_WORDSIZE_8;
    spi_conf.polarity = 0;
    spi_conf.phase = spi_conf.polarity;
    spi_conf.cs = 0;
    spi_conf.itf = 0;
    spi_conf.big_endian = 1;

    pi_open_from_conf(&qspi_dev0, &spi_conf);
    int ret = pi_spi_open(&qspi_dev0);

    g_buf[0] = 0x9F; // jedec id: spi mode
    pi_spi_send(&qspi_dev0, (void*)&g_buf[0], 8, PI_SPI_LINES_SINGLE | PI_SPI_CS_KEEP);
    pi_spi_receive(&qspi_dev0, (void*)&g_buf[0], 8*3, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    printf("jdec id reg = %x:%x:%x\n",g_buf[0],g_buf[1],g_buf[2]);
    
    g_buf[0] = QSPIF_WR_EN_CMD;
    pi_spi_send(&qspi_dev0, (void*)g_buf, 8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    pi_spi_send(&qspi_dev0, (void*)g_set_qspif_dummy, 2*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    g_buf[0] = 0x61;
    // then QPI mode enter
    pi_spi_send(&qspi_dev0, (void*)g_enter_qpi_mode, 1*8, PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    // flash is now in QPI mode, with 2 dummy read cycles
    
    // Try to read id in QSPI mode to ensure switch worked
    g_buf[0] = 0xAF; //jedec id: qspi mode
    pi_spi_send(&qspi_dev0, (void*)&g_buf[0], 8, PI_SPI_LINES_QUAD | PI_SPI_CS_KEEP);
    pi_spi_receive(&qspi_dev0, (void*)&g_buf[0], 8*3, PI_SPI_LINES_QUAD | PI_SPI_CS_AUTO);
    printf("jdec id reg QPI = %x:%x:%x\n",g_buf[0],g_buf[1],g_buf[2]);

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
    g_buf[14] = 0x10;
    g_buf[15] = 0;
    g_buf_u32[4] = SPI_CMD_DUMMY(DUMMY_CYCLES);
    g_buf_u32[5] = SPI_CMD_RX_DATA(len, 1, 0);
    g_buf_u32[6] = SPI_CMD_EOT(1);

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
    int ret = single_lanes_test();
    ret |= qpi_flash_test();
    pmsis_exit(ret);
    return 0;
}
