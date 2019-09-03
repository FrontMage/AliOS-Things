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

#include "driver/uart.h"

#define HAL_WAIT_FOREVER 0xFFFFFFFFU


uint8_t receive_bytes[22];

int application_start(int argc, char *argv[])
{
    printf("uart sample application started...\n");

    // use two stop bits just in case
    uart_config_t uart_config;
    uart_config.baud_rate = 115200; 
    uart_config.mode = MODE_TX_RX; 
    uart_config.stop_bits = STOP_BITS_1; 
    uart_dev_t uart;
    uart.port = 0;
    uart.config = uart_config;
    hal_uart_init(&uart);

    int32_t ret = -1;
    uint32_t i, recv_size = 0;
    while (1)
    {
    #if 1
        for(int i=0; i<20; i++)
        {
            ret = hal_uart_recv(&uart, &receive_bytes[i], 1, HAL_WAIT_FOREVER);
        }

        // en:return by the way you came  ch: 原路返回数据
        for(int i = 0; i<20; i++)
        {
            hal_uart_send(&uart, &receive_bytes[i], 1, HAL_WAIT_FOREVER);
        }
    #else
        hal_uart_recv(&uart, &receive_bytes[i], 1, HAL_WAIT_FOREVER);
        hal_uart_send(&uart, &receive_bytes[i], 1, HAL_WAIT_FOREVER);
    #endif
    }
    return 0;
}
