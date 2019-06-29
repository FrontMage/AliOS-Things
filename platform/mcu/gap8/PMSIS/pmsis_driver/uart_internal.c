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

#include "pmsis_hal/udma/udma_uart.h"
#include "uart/uart.h"
#include "rtos/event_kernel/event_kernel.h"
#include "rtos/os_frontend_api/pmsis_task.h"
#include "rtos/malloc/pmsis_l2_malloc.h"
#include "pmsis_driver/pmsis_fc_event.h"
#include "pmsis_driver/uart_internal.h"


struct uart_driver_data *__global_uart_drv_data[NB_UART];

/*******************************************************************************
 * Prototypes & inlines
 ******************************************************************************/

__attribute__((section(".text"))) __noinline
void uart_handler(void *arg)
{
#ifdef __GAP8__
    uint32_t uart_id = 0;
#else
    uint32_t uart_id = (uint32_t)arg;
#endif
    int irq = __disable_irq();
    int cont =1;
    //while(cont)
    {
        pi_task_t *task;
        hal_compiler_barrier();
        if((uint32_t)arg == UDMA_EVENT_UART_RX)
        {
            task = __uart_drv_fifo_pop_rx(__global_uart_drv_data[uart_id]);
        }
        else
        {
            task = __uart_drv_fifo_pop(__global_uart_drv_data[uart_id]);
        }

        if(task == NULL)
        {
            return;
        }

        if(task->id == PI_TASK_NONE_ID)
        {
            cont = 1;
            pi_task_release(task);
        }
        else
        {
            cont = 0;
            if(task->id == PI_TASK_CALLBACK_ID)
            {
                pmsis_event_push(pmsis_event_get_default_scheduler(), task);
            }
        }
        hal_compiler_barrier();
    }
    __restore_irq(irq);
}

/***********
 * Inner functions
 ***********/

/*!
 * @brief Gets the default configuration structure.
 *
 * This function initializes the UART configuration structure to a default value. The default
 * values are as follows.
 *   pi_uart_conf->baudRate_Bps = 115200U;
 *   pi_uart_conf->parity_mode = UART_PARITY_DISABLED;
 *   pi_uart_conf->stop_bit_count = UART_ONE_STOP_BIT;
 *   pi_uart_conf->enable_tx = false;
 *   pi_uart_conf->enable_rx = false;
 *
 * @param config Pointer to configuration structure.
 */
void pi_uart_conf_init(struct pi_uart_conf *conf)
{
    memset(conf, 0, sizeof(struct pi_uart_conf));
    conf->uart_id = 0;
    conf->baudrate_bps = 115200U;
    conf->parity_mode  = UART_PARITY_DISABLED;

    conf->stop_bit_count = UART_ONE_STOP_BIT;

    conf->enable_tx = 0;
    conf->enable_rx = 0;
}

void  __pi_uart_write_callback(void *arg)
{
    struct uart_callback_args *cb_args = (struct uart_callback_args *)arg;
    int irq = __disable_irq();
    // have to block irq for a short time here as driver is irq driven
    __uart_drv_fifo_enqueue(cb_args->data, cb_args->callback);
    // Enqueue transfer directly since no one is using uart
    udma_uart_enqueue_transfer(cb_args->data->uart_id, cb_args->buffer, cb_args->size, TX_CHANNEL);
    __restore_irq(irq);

    pmsis_l2_malloc_free(cb_args, sizeof(struct uart_callback_args));
}

void  __pi_uart_read_callback(void *arg)
{
    struct uart_callback_args *cb_args = (struct uart_callback_args *)arg;

    int irq = __disable_irq();
    // have to block irq for a short time here as driver is irq driven
    __uart_drv_fifo_enqueue(cb_args->data, cb_args->callback);
    // Enqueue transfer directly since no one is using uart
    udma_uart_enqueue_transfer(cb_args->data->uart_id, cb_args->buffer, cb_args->size, RX_CHANNEL);
    __restore_irq(irq);

    pmsis_l2_malloc_free(cb_args, sizeof(struct uart_callback_args));
}

// Inner function with actual write logic
int __pi_uart_write(struct uart_driver_data *data, void *buffer, uint32_t size, pi_task_t *callback)
{
    // Due to udma restriction, we need to use an L2 address,
    // Since the stack is probably in FC tcdm, we have to either ensure users gave
    // us an L2 pointer or alloc ourselves
    if(((uintptr_t)buffer & 0xFFF00000) != 0x1C000000)
    {
        exit(0);
        return -1;
    }

    // have to block irq for a short time here as driver is irq driven
    int irq = __disable_irq();
    if(__uart_drv_fifo_not_empty(data) && 0)
    {
        __uart_drv_fifo_enqueue(data,__uart_prepare_write_callback(data, buffer, size, callback));
    }
    else
    {
        __uart_drv_fifo_enqueue(data, callback);
        // Enqueue transfer directly since no one is using uart
        udma_uart_enqueue_transfer(data->uart_id, buffer, size, TX_CHANNEL);
    }
    __restore_irq(irq);
    return 0;
}

int __pi_uart_read(struct uart_driver_data *data, void *buffer, uint32_t size, pi_task_t *callback)
{ 
    // Due to udma restriction, we need to use an L2 address,
    // Since the stack is probably in FC tcdm, we have to either ensure users gave
    // us an L2 pointer or alloc ourselves
    if(((uintptr_t)buffer & 0xFFF00000) != 0x1C000000)
    {
        return -1;
    }   

    // have to block irq for a short time here as driver is irq driven
    int irq = __disable_irq();
    if(__uart_drv_fifo_not_empty(data) && 0)
    {
        printf("oups!\n");
        __uart_drv_fifo_enqueue(data, __uart_prepare_read_callback(data, buffer, size, callback));
    }
    else
    {
        __uart_drv_fifo_enqueue_rx(data, callback);
        udma_uart_enqueue_transfer(data->uart_id, buffer, size, RX_CHANNEL);
    }
    __restore_irq(irq);
    return 0;
}
