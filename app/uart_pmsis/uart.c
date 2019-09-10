/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 * 
 * 
 *   this is a uart sample from hal uart for esp8266, and the same as other chips;
 *   for esp8266 , when the port set is 1 ,then the uart1 ande uart2 is the same baud_rate , but the uart1（gpio2） is for log ;
 * 
 *   这sample是一个从hal层适配esp8266的，也许同样适配其他芯片
 *   对于esp8266，如果设置串口为1，则串口一和串口零都是这个波特率，如果需要不一样，请自行修改driver ，注意串口一（gpio2）是作为日志打印。
 * 
 *   Contributor:  https://github.com/xuhongv
 * 
 * 
 */


#include "aos/kernel.h"

#include "aos/hal/uart.h"
#include "pmsis.h"
#define HAL_WAIT_FOREVER 0xFFFFFFFFU


PI_L2 uint8_t receive_bytes[22];

int application_start(int argc, char *argv[])
{
    printf("uart sample application started...\n");

    // use two stop bits just in case
    struct pi_uart_conf uart_config;
    pi_device_t uart;
    memset(&uart,0,sizeof(pi_device_t));
    pi_uart_conf_init(&uart_config);
    uart_config.enable_tx = 1;
    uart_config.enable_rx = 1;
    uart_config.src_clock_Hz = SystemCoreClock;
    pi_open_from_conf(&uart,&uart_config);
    pi_uart_open(&uart);

    int32_t ret = -1;
    uint32_t i, recv_size = 0;
    while (1)
    {
    #if 1
        for(int i=0; i<20; i++)
        {
            ret = pi_uart_read(&uart, &receive_bytes[i], 1);
        }

        // en:return by the way you came  ch: 原路返回数据
        for(int i = 0; i<20; i++)
        {
            pi_uart_write(&uart, &receive_bytes[i], 1);
        }
    #else
        hal_uart_recv(&uart, &receive_bytes[i], 1, HAL_WAIT_FOREVER);
        hal_uart_send(&uart, &receive_bytes[i], 1, HAL_WAIT_FOREVER);
    #endif
    }
    return 0;
}
