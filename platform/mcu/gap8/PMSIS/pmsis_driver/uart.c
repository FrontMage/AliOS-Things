/*
 * Copyright (c) 2019, GreenWaves Technologies, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of GreenWaves Technologies, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "uart/uart.h"
#include "pmsis_hal/udma/udma_uart.h"
#include "pmsis_driver/pmsis_fc_event.h"
#include "pmsis_driver/uart_internal.h"
#include "rtos/event_kernel/event_kernel.h"
#include "rtos/os_frontend_api/pmsis_task.h"
#include "rtos/malloc/pmsis_l2_malloc.h"

#ifdef FEATURE_CLUSTER
#include "pmsis_cluster/cluster_sync/cl_to_fc_delegate.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

int pi_uart_open(pi_device_t *device)
{

    struct pi_uart_conf *conf = device->config;

    if(!__global_uart_drv_data[conf->uart_id])
    {
        struct uart_driver_data *data = pmsis_l2_malloc(sizeof(struct uart_driver_data));
        memset(data,0,sizeof(struct uart_driver_data));

        data->uart_id = conf->uart_id;
        device->data = data;

        // prepare handler here
        pi_fc_event_handler_set(UDMA_EVENT_UART_RX, uart_handler);
        pi_fc_event_handler_set(UDMA_EVENT_UART_TX, uart_handler);
        hal_soc_eu_set_fc_mask(UDMA_EVENT_UART_RX);
        hal_soc_eu_set_fc_mask(UDMA_EVENT_UART_TX);
        // ------------------------

        // init physical uart
        udma_uart_init(conf->uart_id, conf->stop_bit_count,
        conf->parity_mode, conf->enable_tx, conf->enable_rx,
        conf->baudrate_bps, conf->src_clock_Hz);
        __global_uart_drv_data[conf->uart_id] = data;
        data->uart_open_nb = 1;
    }
    else
    {
        struct uart_driver_data *data = __global_uart_drv_data[conf->uart_id];
        data->uart_open_nb++;
        device->data = (void*) data;
    }
    return 0;
}

int pi_uart_write(pi_device_t *device, void *buffer, uint32_t size)
{
    int ret = 0;
    pi_task_t task_block;

    pi_task_block(&task_block);

    if(__pi_uart_write(device->data, buffer, size, &task_block))
    {
        return -1;
    }

    pi_task_wait_on(&task_block);
    return ret;
}

int pi_uart_write_async(pi_device_t *device, void *buffer, uint32_t size,
        pi_task_t *callback)
{
    return __pi_uart_write(device->data, buffer, size, callback);
}

// FIXME/TODO: add some kind of bufferization?
int pi_uart_write_byte(pi_device_t *device, uint8_t *byte)
{
    int ret = pi_uart_write(device, byte, 1);
    return ret;
}

int pi_uart_write_byte_async(pi_device_t *device, uint8_t *byte, pi_task_t *callback)
{
    return pi_uart_write_async(device, byte, 1, callback);
}

int pi_uart_read(pi_device_t *device, void *buffer, uint32_t size)
{
    // Due to udma restriction, we need to use an L2 address,
    // Since the stack is probably in FC tcdm, we have to either ensure users gave
    // us an L2 pointer or alloc ourselves
    if(((uintptr_t)buffer & 0xFFF00000) != 0x1C000000)
    {
        return -1;
    }

    pi_task_t task_block;
    pi_task_block(&task_block);

    __pi_uart_read(device->data, buffer, size, &task_block);

    pi_task_wait_on(&task_block);
    return 0;
}

int pi_uart_read_async(pi_device_t *device, void *buffer, uint32_t size,
        pi_task_t *callback)
{
    struct uart_driver_data *data = device->data;
    __uart_drv_fifo_enqueue(data, __uart_prepare_read_callback(data, buffer, size, callback));
    return 0;
}

void pi_uart_close(pi_device_t *device)
{
    struct uart_driver_data *data = device->data;
    struct pi_uart_conf *conf = device->config;
    int irq = __disable_irq();
    data->uart_open_nb--;

    if(data->uart_open_nb == 0)
    {
        pmsis_l2_malloc_free(data, sizeof(struct uart_driver_data));
        __global_uart_drv_data[conf->uart_id] = NULL;
    }
    restore_irq(irq);
}
