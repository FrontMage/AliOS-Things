#include "pmsis.h"
#include <aos/hal/uart.h>
#include <errno.h>
#include <stdio.h>
#include "gap_semihost.h"


//#define UART_SEMIHOST_EMUL

uart_dev_t uart_0 = {
    .port = 0,                                                  /* uart port */
    .config = {9600, DATA_WIDTH_8BIT, NO_PARITY, STOP_BITS_1,
        FLOW_CONTROL_DISABLED, MODE_TX_RX},                     /* uart config */
    .priv = NULL                                                /* priv data */
};

int32_t hal_uart_init(uart_dev_t *uart)
{
#if !defined(UART_SEMIHOST_EMUL)
    if(!__global_uart_drv_data[uart->port])
    {
        struct uart_driver_data *data = pi_default_malloc(sizeof(struct uart_driver_data));
        
        uart->priv = data;
        data->uart_id = uart->port;
        data->uart_fifo_head[0] = NULL;
        data->uart_fifo_tail[0] = NULL;
        data->uart_fifo_head[1] = NULL;
        data->uart_fifo_tail[1] = NULL;

        struct pi_uart_conf conf;
        pi_uart_conf_init(&conf);

        switch(uart->config.mode)
        {
            case MODE_TX:
                conf.enable_tx = 1;
                conf.enable_rx = 0;
                break;
            case MODE_RX:
                conf.enable_tx = 0;
                conf.enable_rx = 1;
                break;
            case MODE_TX_RX:
                conf.enable_tx = 1;
                conf.enable_rx = 1;
                break;
            default:
                return EIO;
                break;
        }

        switch(uart->config.stop_bits)
        {
            case STOP_BITS_1:
                conf.stop_bit_count = UART_ONE_STOP_BIT;
                break;
            case STOP_BITS_2:
                conf.stop_bit_count = UART_TWO_STOP_BIT;
                break;
            default:
                return EIO;
                break;
        }

        // FIXME: data width ignored for now, TODO:check RTL

        conf.baudrate_bps = uart->config.baud_rate;
        conf.src_clock_Hz = system_core_clock_get();

        // prepare handler here
        pi_fc_event_handler_set(UDMA_EVENT_UART_RX, uart_handler);
        pi_fc_event_handler_set(UDMA_EVENT_UART_TX, uart_handler);
        hal_soc_eu_set_fc_mask(UDMA_EVENT_UART_RX);
        hal_soc_eu_set_fc_mask(UDMA_EVENT_UART_TX);

        // init physical uart
        udma_uart_init(conf.uart_id, conf.stop_bit_count,
                conf.parity_mode, conf.enable_tx, conf.enable_rx,
                conf.baudrate_bps, conf.src_clock_Hz);
        data->uart_open_nb = 1;

        krhino_mutex_create(&data->uart_mutex_rx, "uart_mutex_rx");
        krhino_mutex_create(&data->uart_mutex_tx, "uart_mutex_tx");

        krhino_mutex_unlock(&data->uart_mutex_rx);
        krhino_mutex_unlock(&data->uart_mutex_tx);

        // ---------------
        // setup rx polling
        // TODO: to achieve higher baudrate
        // ++++++ avoid any blocking on other threads
        // ++++++ manage first level fifo (circular buffer?)
        // ---------------
        __global_uart_drv_data[conf.uart_id] = data;
    }
    else
    {
        struct uart_driver_data *data = __global_uart_drv_data[uart->port];
        data->uart_open_nb++;
        uart->priv = (void*) data;
    }
#endif
    return 0;
}


int32_t hal_uart_send(uart_dev_t *uart, const void *data, uint32_t size, uint32_t timeout)
{
#if !defined(UART_SEMIHOST_EMUL)
    // copy the buffer to L2 if need be -- if app is not made with gap in mind
    if(((uintptr_t)data & 0xFFF00000) != 0x1C000000)
    {
        void *l2_buff = pmsis_l2_malloc(size);
        if(!l2_buff)
        {
            return EIO;
        }
        memcpy(l2_buff, data, size);
        pi_task_t task_block;
        pi_task_block(&task_block);
        __pi_uart_write(uart->priv, l2_buff, size, &task_block);
        pi_task_wait_on(&task_block);
        pi_task_destroy(&task_block);
        pi_l2_free(l2_buff, size);
    }
    else
    {
        pi_task_t task_block;
        pi_task_block(&task_block);
        __pi_uart_write(uart->priv, (void*)data, size, &task_block);
        pi_task_wait_on(&task_block);
        pi_task_destroy(&task_block);
    }
    return 0;
#else
    return gap8_semihost_write(1, data, size);
#endif
}

int32_t hal_uart_recv(uart_dev_t *uart, void *data, uint32_t expect_size, uint32_t timeout)
{
#if !defined(UART_SEMIHOST_EMUL)
    pi_task_t task_block;
    pi_task_block(&task_block);
    // copy the buffer to L2 if need be -- if app is not made with gap in mind
    if (((uintptr_t)data & 0xFFF00000) != 0x1C000000)
    {
        void *l2_buff = pi_l2_malloc(expect_size);
        ;
        if (!l2_buff)
        {
            return EIO;
        }
        __pi_uart_read(uart->priv, l2_buff, expect_size, &task_block);
        pi_task_wait_on(&task_block);
        memcpy(data, l2_buff, expect_size);
        pi_l2_free(l2_buff, expect_size);
    }
    else
    {
        __pi_uart_read(uart->priv, data, expect_size, &task_block);
        pi_task_wait_on(&task_block);
    }

    pi_task_destroy(&task_block);
    return 0;
    #else

    return gap8_semihost_read(0, data, expect_size);
    #endif
}

int32_t hal_uart_recv_II(uart_dev_t *uart, void *data, uint32_t expect_size,
                         uint32_t *recv_size, uint32_t timeout)
{
#if !defined(UART_SEMIHOST_EMUL)
    if(uart->priv == NULL)
    {
        uart->priv = uart_0.priv;
        uart->config = uart_0.config;
        //printf("priv=%p, config=%p, data size=%d\n",uart->priv, uart->config,sizeof(struct uart_driver_data));
    }
    krhino_mutex_lock(&((struct uart_driver_data*)uart->priv)->uart_mutex_rx, HAL_WAIT_FOREVER);
    pi_task_t task_block;
    pi_task_block(&task_block);
    // copy the buffer to L2 if need be -- if app is not made with gap in mind
    if(((uintptr_t)data & 0xFFF00000) != 0x1C000000)
    {
        void *l2_buff = pi_l2_malloc(expect_size);;
        if(!l2_buff)
        {
            krhino_mutex_unlock(&((struct uart_driver_data*)uart->priv)->uart_mutex_rx);
            return EIO;
        }
        int ret = __pi_uart_read(uart->priv, l2_buff, expect_size, &task_block);
        pi_task_wait_on(&task_block);
        memcpy(data, l2_buff, expect_size);
        pi_l2_free(l2_buff, expect_size);
    }
    else
    {
        __pi_uart_read(uart->priv, data, expect_size, &task_block);
        pi_task_wait_on(&task_block);
    }

    pi_task_destroy(&task_block);
    // udma does not support partial transfers, so if we're here, we got everything
    *recv_size = expect_size;
    krhino_mutex_unlock(&((struct uart_driver_data*)uart->priv)->uart_mutex_rx);
    return 0;
#else
    int ret = gap8_semihost_read(0, data, expect_size);
    *recv_size = expect_size;
    return ret;
#endif
}

int32_t hal_uart_finalize(uart_dev_t *uart)
{
    return 0;
}
