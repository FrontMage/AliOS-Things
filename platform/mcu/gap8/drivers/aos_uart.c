#include "pmsis.h"
#include "pmsis_driver/uart_internal.h"
#include <aos/hal/uart.h>
#include <errno.h>

uart_dev_t uart_0 = {
    .port = 0,                                                                          /* uart port */
    .config = {115200, DATA_WIDTH_8BIT, NO_PARITY, STOP_BITS_1, FLOW_CONTROL_DISABLED}, /* uart config */
    .priv = NULL                                                                        /* priv data */
};

int32_t hal_uart_init(uart_dev_t *uart)
{
    if(!__global_uart_drv_data[uart->port])
    {
        struct uart_driver_data *data = pmsis_l2_malloc(sizeof(struct uart_driver_data));

        uart_0.priv = data;
        data->uart_id = uart->port;
        data->uart_fifo_head = NULL;
        data->uart_fifo_tail = NULL;

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

        conf.baudrate_bps = uart->config.baud_rate;;
        conf.src_clock_Hz = SystemCoreClock;

        // prepare handler here
        pi_fc_event_handler_set(UDMA_EVENT_UART_RX, uart_handler);
        pi_fc_event_handler_set(UDMA_EVENT_UART_TX, uart_handler);
        hal_soc_eu_set_fc_mask(UDMA_EVENT_UART_RX);
        hal_soc_eu_set_fc_mask(UDMA_EVENT_UART_TX);

        // init physical uart
        udma_uart_init(conf.uart_id, conf.stop_bit_count,
                conf.parity_mode, conf.enable_tx, conf.enable_rx,
                conf.baudrate_bps, conf.src_clock_Hz);
        __global_uart_drv_data[conf.uart_id] = data;
        data->uart_open_nb = 1;
    }
    else
    {
        struct uart_driver_data *data = __global_uart_drv_data[uart->port];
        data->uart_open_nb++;
        uart->priv = (void*) data;
    }

    return 0;
}


int32_t hal_uart_send(uart_dev_t *uart, const void *data, uint32_t size, uint32_t timeout)
{
    pi_task_t task_block;
    pi_task_block(&task_block);
    // copy the buffer to L2 if need be -- if app is not made with gap in mind
    if(((uintptr_t)data & 0xFFF00000) != 0x1C000000)
    {
        void *l2_buff = pmsis_l2_malloc(size);;
        if(!l2_buff)
        {
            return EIO;
        }
        memcpy(l2_buff, data, size);
        __pi_uart_write(uart->priv, l2_buff, size, &task_block);
    }
    else
    {
        __pi_uart_write(uart->priv, data, size, &task_block);
    }

    pi_task_wait_on(&task_block);
    pi_task_destroy(&task_block);
    return 0;
}

int32_t hal_uart_recv(uart_dev_t *uart, void *data, uint32_t expect_size, uint32_t timeout)
{
    return 0;
}

int32_t hal_uart_recv_II(uart_dev_t *uart, void *data, uint32_t expect_size,
                         uint32_t *recv_size, uint32_t timeout)
{
    return 0;
}

int32_t hal_uart_finalize(uart_dev_t *uart)
{
    return 0;
}
