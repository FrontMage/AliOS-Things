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

#include "stddef.h"
#include "stdarg.h"
#include "pmsis_os.h"
#include "pmsis_eu.h"
#include "pmsis_fc_event.h"
#include "udma_hyper.h"
#include "hyperbus.h"
#include "cl_to_fc_delegate.h"
#include "gap_common.h"
#include "pinmap.h"
#include "PeripheralPins.h"
#include "pmsis_periph.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifdef DEBUG
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(...) ((void) 0)
#endif  /* DEBUG */

#define __PI_HYPER_TEMP_BUFFER_SIZE 128
#define BURST_SIZE 512

/*******************************************************************************
 * Driver data
 *****************************************************************************/

/* For burst. */
udma_channel_e pending_channel;
uint32_t pending_device_id;
uint32_t pending_hyper_addr;
uint32_t pending_buffer;
uint32_t pending_repeat;
uint32_t pending_repeat_size;

/* Emulated transfers. */
udma_channel_e pending_emu_channel;
uint32_t pending_emu_device_id;
uint32_t pending_emu_hyper_addr;
uint32_t pending_emu_buffer;
uint32_t pending_emu_size;
uint32_t pending_emu_size_2d;
uint32_t pending_emu_length;
uint32_t pending_emu_stride;
uint32_t pending_emu_do_memcpy;
struct pi_task *pending_emu_task;

struct hyper_driver_task
{
    pi_task_t *task;
    struct hyper_driver_task *next;
};

struct hyper_driver_fifo
{
    struct hyper_driver_task *fifo_head;
    struct hyper_driver_task *fifo_tail;
};

struct hyper_driver_fifo *__global_hyper_driver_fifo[UDMA_NB_HYPER];

static struct pi_task *__pi_hyper_end_task;

static uint8_t __pi_hyper_temp_buffer[__PI_HYPER_TEMP_BUFFER_SIZE];

static uint8_t hyper_periph_init = 0, hyper_ram_init = 0, hyper_flash_init = 0;

#if 0
struct hyper_transfer_s
{
    uint32_t hyper_addr;
    void *buffer;
    uint32_t size;
    uint32_t stride;
    uint32_t length;
    udma_channel_e channel;
    int8_t device_id;
};

struct hyper_cb_args_s
{
    struct pi_task *cb;
    struct hyper_transfer_s transfer;
};

#if (FEATURE_CLUSTER == 1)
struct pi_cl_hyper_req_s
{
    struct pi_device *device;
    struct hyper_transfer_s transfer;
    struct pi_cl_hyper_req_s *next;
    pi_task_t task_done;
    uint8_t cid;
    uint8_t is_2d;
};

struct pi_cl_hyperram_alloc_req_s
{
    struct pi_device *device;
    uint32_t result;
    uint32_t size;
    pi_task_t task_done;
    uint8_t cid;
};

struct pi_cl_hyperram_free_req_s
{
    struct pi_device *device;
    uint32_t result;
    uint32_t size;
    uint32_t chunk;
    pi_task_t task_done;
    uint8_t cid;
};
#endif  /* (FEATURE_CLUSTER == 1) */
#endif
/*******************************************************************************
 * Function declaration
 ******************************************************************************/

static void hyper_handler(void *arg);
static void __pi_hyper_handle_end_of_task(struct pi_task *task);
static inline void *l2_memcpy(void *dst0, const void *src0, size_t len0);
static uint32_t __pi_hyper_alloc_init(malloc_t *alloc, uint32_t heapstart, int32_t size);
static void __pi_hyper_alloc_deinit(malloc_t *alloc);
/* Enqueue a task in fifo. */
static void __pi_hyper_task_fifo_enqueue(struct hyper_driver_fifo *fifo, struct pi_task *task);
/* Pop a task from fifo. */
static struct pi_task *__pi_hyper_task_fifo_pop(struct hyper_driver_fifo *fifo);
static void exec_pending_task();
static void __pi_hyper_callback(void *arg);
static struct pi_task *__pi_hyper_prepare_callback(struct hyper_transfer_s *transfer,
                                                   struct pi_task *task);
static void __pi_hyper_copy_aligned(struct hyper_transfer_s *transfer, struct pi_task *task);
static int32_t __pi_hyper_resume_misaligned_write(struct pi_task *task);
static int32_t __pi_hyper_resume_misaligned_read(struct pi_task *task);
static void __pi_hyper_copy_misaligned(struct pi_task *task);
static void __pi_hyper_resume_emu_task();
static void __pi_hyper_copy_exec(struct hyper_transfer_s *transfer, struct pi_task *task);
static void __pi_hyper_copy_2d(int device, struct hyper_transfer_s *transfer,
                            struct pi_task *task);
static void __pi_hyper_copy(int device, struct hyper_transfer_s *transfer,
                            struct pi_task *task);


/*******************************************************************************
 * Inner functions.
 ******************************************************************************/

/* TODO : In ASM, maybe ? */

static void __pi_hyper_handle_end_of_task(struct pi_task *task)
{
    if(task->id == PI_TASK_NONE_ID)
    {
        pi_task_release(task);
    }
    else
    {
        if(task->id == PI_TASK_CALLBACK_ID)
        {
            pmsis_event_push(pmsis_event_get_default_scheduler(), task);
        }
    }
}

static void hyper_handler(void *arg)
{
    if (pending_repeat != 0)
    {
        pending_buffer += pending_repeat;
        pending_hyper_addr += pending_repeat;
        pending_repeat_size -= pending_repeat;

        uint32_t iter_size = pending_repeat;
        if (pending_repeat_size <= pending_repeat)
        {
            pending_repeat = 0;
            iter_size = pending_repeat_size;
        }
        hal_hyper_enqueue(pending_device_id, pending_channel, pending_hyper_addr,
                          pending_buffer, iter_size, UDMA_CFG_EN(1));
    }
    else
    {
        /* Case end of current transfer. */
        struct pi_task *task = __pi_hyper_end_task;
        if (task != NULL)
        {
            __pi_hyper_end_task = NULL;
            __pi_hyper_handle_end_of_task(task);
        }
        /* Case pending misaligned transfer. */
        if (pending_emu_task != NULL)
        {
            __pi_hyper_resume_emu_task();
            return;
        }
        /* Case new transfer in fifo. */
        /* Maybe use a struct to get fifo of tasks. */
        task = __pi_hyper_task_fifo_pop(__global_hyper_driver_fifo[0]);
        if (task != NULL)
        {
            __pi_hyper_handle_end_of_task(task);
        }
    }
}

static inline void *l2_memcpy(void *dst0, const void *src0, size_t len0)
{
  char *dst = (char *) dst0;
  char *src = (char *) src0;

  void *save = dst0;

  while (len0--)
  {
      *dst++ = *src++;
  }

  return save;
}

static uint32_t __pi_hyper_alloc_init(malloc_t *alloc, uint32_t heapstart, int32_t size)
{
    return __malloc_extern_init(alloc, 0, size);
}

static void __pi_hyper_alloc_deinit(malloc_t *alloc)
{
    __malloc_extern_deinit(alloc);
    pmsis_l2_malloc_free(alloc, sizeof(malloc_t));
}

/* Enqueue a task in fifo. */
static void __pi_hyper_task_fifo_enqueue(struct hyper_driver_fifo *fifo, struct pi_task *task)
{
    uint32_t irq = __disable_irq();
    struct hyper_driver_task *new_task = (struct hyper_driver_task *) pmsis_l2_malloc(sizeof(struct hyper_driver_task));
    if (new_task == NULL)
    {
        __restore_irq(irq);
        return;
    }
    new_task->task = task;
    new_task->next = NULL;
    if (fifo->fifo_head == NULL)
    {
        /* Empty fifo. */
        fifo->fifo_head = new_task;
        fifo->fifo_tail = fifo->fifo_head;
    }
    else
    {
        fifo->fifo_tail->next = new_task;
        fifo->fifo_tail = fifo->fifo_tail->next;
    }
    __restore_irq(irq);
}

/* Pop a task from fifo. */
static struct pi_task *__pi_hyper_task_fifo_pop(struct hyper_driver_fifo *fifo)
{
    uint32_t irq = __disable_irq();
    struct pi_task *task_to_return = NULL;
    if (fifo->fifo_head != NULL)
    {
        task_to_return = fifo->fifo_head->task;
        struct hyper_driver_task *task_to_free = fifo->fifo_head;
        fifo->fifo_head = fifo->fifo_head->next;
        pmsis_l2_malloc_free(task_to_free, sizeof(struct hyper_driver_task));
        if (fifo->fifo_head == NULL)
        {
            fifo->fifo_tail = NULL;
        }
    }
    __restore_irq(irq);
    return task_to_return;
}

/* TODO : Change prot to take a fifo in arg. */
/* Pass struct device for device_id. */
static void exec_pending_task()
{
    struct pi_task *task = __pi_hyper_task_fifo_pop(__global_hyper_driver_fifo[0]);

    if (task != NULL)
    {
        __pi_hyper_handle_end_of_task(task);
    }
}

static void __pi_hyper_callback(void *arg)
{
    struct hyper_cb_args_s *cb_args = (struct hyper_cb_args_s *) arg;

    if (cb_args->transfer.stride)
    {
        __pi_hyper_copy_2d(cb_args->transfer.device_id, &cb_args->transfer, cb_args->cb);
    }
    else
    {
        __pi_hyper_copy(cb_args->transfer.device_id, &cb_args->transfer, cb_args->cb);
    }

    /* Free used structures. */
    pmsis_l2_malloc_free(cb_args, sizeof(struct hyper_cb_args_s));
}

static struct pi_task *__pi_hyper_prepare_callback(struct hyper_transfer_s *transfer,
                                                   struct pi_task *task)
{
    struct pi_task *cb_task = (struct pi_task *) pmsis_l2_malloc(sizeof(struct pi_task));
    struct hyper_cb_args_s *cb_args = (struct hyper_cb_args_s *) pmsis_l2_malloc(sizeof(struct hyper_cb_args_s));
    if ((cb_task == NULL) || (cb_args == NULL))
    {
        return NULL;
    }
    /* Callback args. */
    cb_args->transfer.hyper_addr = transfer->hyper_addr;
    cb_args->transfer.buffer = transfer->buffer;
    cb_args->transfer.size = transfer->size;
    cb_args->transfer.channel = transfer->channel;
    cb_args->transfer.device_id = transfer->device_id;
    cb_args->transfer.stride = transfer->stride;
    cb_args->transfer.length = transfer->length;
    cb_args->cb = task;
    /* Callback task. */
    pi_task_callback_no_mutex(cb_task,__pi_hyper_callback,cb_args);
    cb_task->destroy = 1;

    return cb_task;
}

static void __pi_hyper_copy_aligned(struct hyper_transfer_s *transfer, struct pi_task *task)
{
    if (transfer->size > (uint32_t) BURST_SIZE)
    {
        pending_channel = transfer->channel;
        pending_device_id = transfer->device_id;
        pending_hyper_addr = transfer->hyper_addr;
        pending_buffer = (uint32_t) transfer->buffer;
        pending_repeat = (uint32_t) BURST_SIZE;
        pending_repeat_size = transfer->size;
        transfer->size = (uint32_t) BURST_SIZE;
    }
    else
    {
        pending_repeat = 0;
    }
    __pi_hyper_end_task = task;
    hal_hyper_enqueue(transfer->device_id, transfer->channel, transfer->hyper_addr,
                      (uint32_t) transfer->buffer, transfer->size, UDMA_CFG_EN(1));
}

static void __pi_hyper_copy_exec(struct hyper_transfer_s *transfer, struct pi_task *task)
{
    if ((((uint32_t) transfer->buffer & 0x3) == 0) &&
        (((uint32_t) transfer->hyper_addr & 0x1) == 0) &&
        ((((uint32_t) transfer->size & 0x3) == 0) ||
         ((transfer->channel == TX_CHANNEL) && (((uint32_t) transfer->size & 0x1) == 0))))
    {
        /* Aligned copy. */
        __pi_hyper_copy_aligned(transfer, task);
    }
    else
    {
        /* Misaligned copy. */
        pending_emu_channel = transfer->channel;
        pending_emu_device_id = transfer->device_id;
        pending_emu_hyper_addr = transfer->hyper_addr;
        pending_emu_buffer = (uint32_t) transfer->buffer;
        pending_emu_size = transfer->size;
        pending_emu_do_memcpy = 0;
        pending_emu_task = task;
        __pi_hyper_copy_misaligned(task);
    }
}


static void __pi_hyper_copy_2d_exec(struct hyper_transfer_s *transfer, struct pi_task *task)
{
    /* Misaligned copy. */
    pending_emu_channel = transfer->channel;
    pending_emu_device_id = transfer->device_id;
    pending_emu_hyper_addr = transfer->hyper_addr;
    pending_emu_buffer = (uint32_t) transfer->buffer;
    pending_emu_size = (transfer->size > transfer->length) ? transfer->length : transfer->size;
    pending_emu_do_memcpy = 0;
    pending_emu_size_2d = transfer->size;
    pending_emu_length = transfer->length;
    pending_emu_stride = transfer->stride;
    pending_emu_task = task;
    __pi_hyper_copy_misaligned(task);
}

static void __pi_hyper_copy(int device, struct hyper_transfer_s *transfer,
                            struct pi_task *task)
{
    uint32_t irq = __disable_irq();
    if ((__pi_hyper_end_task != NULL) || (pending_emu_size != 0))
    {
        /* Transfer on going, enqueue this one to the list. */
        struct pi_task *task_enqueue = __pi_hyper_prepare_callback(transfer, task);
        __pi_hyper_task_fifo_enqueue(__global_hyper_driver_fifo[device], task_enqueue);
    }
    else
    {
        /* Execute the transfer. */
        __pi_hyper_copy_exec(transfer, task);
    }
    __restore_irq(irq);
}

void __pi_hyper_copy_2d(int device, struct hyper_transfer_s *transfer,
                        struct pi_task *task)
{
    uint32_t irq = __disable_irq();
    if ((__pi_hyper_end_task != NULL) || (pending_emu_size_2d != 0))
    {
        /* Transfer on going, enqueue this one to the list. */
        struct pi_task *task_enqueue = __pi_hyper_prepare_callback(transfer, task);
        __pi_hyper_task_fifo_enqueue(__global_hyper_driver_fifo[device], task_enqueue);
    }
    else
    {
        /* Execute the transfer. */
        __pi_hyper_copy_2d_exec(transfer, task);
    }
    __restore_irq(irq);
}


/* Hyper settings. */
static void __pi_hyper_settings(uint8_t chip_select)
{
    /* MBR0 is Ram, MBR1 is Flash. */
    if (!chip_select)
    {
        hyper_mbr0_set(REG_MBR0);
        hyper_mbr1_set(REG_MBR1 >> 24);
        hyper_dt0_set(PI_HYPER_TYPE_RAM);
        hyper_dt1_set(PI_HYPER_TYPE_FLASH);
        hyper_crt0_set(MEM_ACCESS);
        hyper_crt1_set(MEM_ACCESS); /* Flash is always in mem access. */
    }
    else
    {
        hyper_mbr0_set(REG_MBR1 >> 24);
        hyper_mbr1_set(REG_MBR0);
        hyper_dt0_set(PI_HYPER_TYPE_FLASH);
        hyper_dt1_set(PI_HYPER_TYPE_RAM);
        hyper_crt0_set(MEM_ACCESS); /* Flash is always in mem access. */
    }
    /* Max length setting. */
    hyper_max_length_set(0x1ff, 1, 0, chip_select);
    hyper_max_length_set(0x1ff, 1, 1, chip_select);
    /* Timing setting. */
    hyper_timing_set(4, 4, 4, 1, 0, chip_select);
    hyper_timing_set(4, 4, 4, 1, 1, chip_select);
}

/* Pin settings. */
static void __pi_hyper_pin_settings(uint32_t nbArgs,...)
{
    PORT_Type *const xPort_addrs[] = PORT_BASE_PTRS;
    port_pin_config_t config = { uPORT_PullUpEnable, uPORT_HighDriveStrength, uPORT_MuxAlt3 };
    PinName pin = 0;
    uint32_t port = 0, pin_nb = 0;

    /* pin out the hyperbus pins */
    va_list list;
    va_start( list, nbArgs );
    for( uint32_t i = 0; i< nbArgs; i++)
    {
        pin = va_arg( list, PinName );
        port = GET_GPIO_PORT( pin );
        pin_nb = GET_GPIO_PIN_NUM( pin );
        PORT_SetPinConfig( xPort_addrs[port], pin_nb, &config );
    }
    va_end( list );
}

/* HyperFlash status register device ready offset. */
#define DEVICE_READY_OFFSET     7
/* Write and read address */
#define SA                      0x0000

/*! @brief HyperFlash command sequence structure. */
typedef struct
{
    uint16_t data;              /*!< Command data. */
    uint16_t addr;              /*!< Commad address. */
} cmdSeq;


/* Sector erase sequence */
static cmdSeq Erase_Seq[6] = {{0xAA, 0x555}, {0x55, 0x2AA}, {0x80, 0x555},
                              {0xAA, 0x555}, {0x55, 0x2AA}, {0x30, SA}};

/* Configure register0 sequence */
static cmdSeq VCR_Seq[4]   = {{0xAA, 0x555}, {0x55, 0x2AA}, {0x38, 0x555}, {0x8e0b, 0x0}};

/* Read status register sequence */
static cmdSeq Reg_Seq      = {0x70, 0x555};

/* Write 512/4 = 128 word to Sector addr 0x4xxx */
static cmdSeq WP_Seq[3]    = {{0xAA, 0x555}, {0x55, 0x2AA}, {0xA0, 0x555}};

struct pi_device *g_hyper_device = NULL;
static uint32_t read_val = 0, write_val = 0;

void __pi_hyper_flash_config(struct pi_device *device);
void __pi_hyper_flash_erase(struct pi_device *device, uint32_t hyper_addr);
void __pi_hyper_flash_write(struct pi_device *device, uint32_t hyper_addr,
                            uint32_t buffer, uint32_t size);
void __pi_hyper_flash_read(struct pi_device *device, uint32_t hyper_addr,
                           uint32_t buffer, uint32_t size);
void __pi_hyper_flash_sync(struct pi_device *device);


void __pi_hyper_flash_config(struct pi_device *device)
{
    pi_hyper_type_e type = ((struct pi_hyper_conf*)device->config)->type;
    ((struct pi_hyper_conf*)device->config)->type = PI_HYPER_TYPE_FLASH;

    for (uint32_t i=0; i<4; i++)
    {
        pi_hyper_write(device, (VCR_Seq[i].addr << 1), &VCR_Seq[i].data, 2);
    }
    ((struct pi_hyper_conf*)device->config)->type = type;
}

void __pi_hyper_flash_erase(struct pi_device *device, uint32_t hyper_addr)
{
    pi_hyper_type_e type = ((struct pi_hyper_conf*)device->config)->type;
    ((struct pi_hyper_conf*)device->config)->type = PI_HYPER_TYPE_FLASH;

    for (uint32_t i=0; i<5; i++)
    {
        pi_hyper_write(device, (Erase_Seq[i].addr << 1), &Erase_Seq[i].data, 2);
    }
    pi_hyper_write(device, hyper_addr, &Erase_Seq[5].data, sizeof(uint16_t));
    ((struct pi_hyper_conf*)device->config)->type = type;
}

void __pi_hyper_flash_write(struct pi_device *device, uint32_t hyper_addr,
                            uint32_t buffer, uint32_t size)
{
    pi_hyper_type_e type = ((struct pi_hyper_conf*)device->config)->type;
    ((struct pi_hyper_conf*)device->config)->type = PI_HYPER_TYPE_FLASH;
    int32_t _size = size;
    uint32_t _hyper_addr = hyper_addr;
    uint32_t _buffer = buffer;
    while(_size > 0)
    {
        uint32_t transf_size = 0;
        if(_size > BURST_SIZE)
        {
             transf_size = BURST_SIZE;
        }
        else
        {
            transf_size = _size;
        }
        for( uint32_t i = 0; i < 3; i++ )
        {
            pi_hyper_write(device, (WP_Seq[i].addr << 1), &WP_Seq[i].data, 2);
        }
        pi_hyper_write(device, _hyper_addr, (void *) _buffer, transf_size);
        _size -= BURST_SIZE;
        _hyper_addr += BURST_SIZE;
        _buffer += BURST_SIZE;
        __pi_hyper_flash_sync(device);
    }
    ((struct pi_hyper_conf*)device->config)->type = type;
}

void __pi_hyper_flash_read(struct pi_device *device, uint32_t hyper_addr,
                           uint32_t buffer, uint32_t size)
{
    pi_hyper_type_e type = ((struct pi_hyper_conf*)device->config)->type;
    ((struct pi_hyper_conf*)device->config)->type = PI_HYPER_TYPE_FLASH;

    int32_t _size = size;
    uint32_t _hyper_addr = hyper_addr;
    uint32_t _buffer = buffer;
    while(_size > 0)
    {
        uint32_t transf_size = 0;
        if(_size > BURST_SIZE)
        {
             transf_size = BURST_SIZE;
        }
        else
        {
            transf_size = _size;
        }
        pi_hyper_read(device, _hyper_addr, (void *) _buffer, transf_size);
        _size -= BURST_SIZE;
        _hyper_addr += BURST_SIZE;
        _buffer += BURST_SIZE;

    }
    ((struct pi_hyper_conf*)device->config)->type = type;
}

void __pi_hyper_flash_sync(struct pi_device *device)
{
    pi_hyper_type_e type = ((struct pi_hyper_conf*)device->config)->type;
    ((struct pi_hyper_conf*)device->config)->type = PI_HYPER_TYPE_FLASH;

    /* Wait the end of process
     * Status Register (SR)
     * bit 4 -> program status bit, 0-success ; 1-failure
     * bit 5 -> erase status bit,   0-success ; 1-failure
     * bit 7 -> device ready bit,   0-busy    ; 1-ready
     */
    uint16_t reg;
    write_val = Reg_Seq.data;
    do
    {
        pi_hyper_write(device, Reg_Seq.addr << 1, &write_val, 2);
        pi_hyper_read(device, 0, &read_val, 2);
        //reg = ( ( read_val >> 16 ) & 0xffff );
        reg = ( read_val );
    } while( !( reg & ( 1 << DEVICE_READY_OFFSET ) ) );
    ((struct pi_hyper_conf*)device->config)->type = type;
}

void pi_hyper_flash_erase(struct pi_device *device, uint32_t hyper_addr);
void pi_hyper_flash_write(struct pi_device *device, uint32_t hyper_addr,
                          uint32_t buffer, uint32_t size);
void pi_hyper_flash_read(struct pi_device *device, uint32_t hyper_addr,
                         uint32_t buffer, uint32_t size);
void pi_hyper_flash_sync(struct pi_device *device);

void pi_hyper_flash_erase(struct pi_device *device, uint32_t hyper_addr)
{
    for (uint32_t i=0; i<5; i++)
    {
        pi_hyper_write(device, (Erase_Seq[i].addr << 1), &Erase_Seq[i].data, 2);
    }
    pi_hyper_write(device, hyper_addr, &Erase_Seq[5].data, sizeof(uint16_t));
}

void pi_hyper_flash_write(struct pi_device *device, uint32_t hyper_addr,
                          uint32_t buffer, uint32_t size)
{
    int32_t _size = size;
    uint32_t _hyper_addr = hyper_addr;
    uint32_t _buffer = buffer;
    while (_size > 0)
    {
        uint32_t transf_size = 0;
        if (_size > BURST_SIZE)
        {
             transf_size = BURST_SIZE;
        }
        else
        {
            transf_size = _size;
        }
        for (uint32_t i=0; i<3; i++)
        {
            pi_hyper_write(device, (WP_Seq[i].addr << 1), &WP_Seq[i].data, 2);
        }
        pi_hyper_write(device, _hyper_addr, (void *) _buffer, transf_size);
        _size -= BURST_SIZE;
        _hyper_addr += BURST_SIZE;
        _buffer += BURST_SIZE;
        pi_hyper_flash_sync(device);
    }
}

void pi_hyper_flash_read(struct pi_device *device, uint32_t hyper_addr,
                         uint32_t buffer, uint32_t size)
{
    int32_t _size = size;
    uint32_t _hyper_addr = hyper_addr;
    uint32_t _buffer = buffer;
    while (_size > 0)
    {
        uint32_t transf_size = 0;
        if (_size > BURST_SIZE)
        {
             transf_size = BURST_SIZE;
        }
        else
        {
            transf_size = _size;
        }
        pi_hyper_read(device, _hyper_addr, (void *) _buffer, transf_size);
        _size -= BURST_SIZE;
        _hyper_addr += BURST_SIZE;
        _buffer += BURST_SIZE;
    }
}

void pi_hyper_flash_sync(struct pi_device *device)
{
    uint16_t reg;
    write_val = Reg_Seq.data;
    do
    {
        pi_hyper_write(device, Reg_Seq.addr << 1, &write_val, 2);
        pi_hyper_read(device, 0, &read_val, 2);
        //reg = ( ( read_val >> 16 ) & 0xffff );
        reg = ( read_val );
    } while( !( reg & ( 1 << DEVICE_READY_OFFSET ) ) );
}

/*******************************************************************************
 * PMSIS FC functions implem
 ******************************************************************************/

void pi_hyper_conf_init(struct pi_hyper_conf *conf)
{
    memset(conf, 0, sizeof(struct pi_hyper_conf));
    conf->device = PI_DEVICE_HYPERBUS_TYPE;
    conf->cs = 0;
    conf->type = PI_HYPER_TYPE_RAM;
    conf->id = 0;
    conf->ram_size = 0;
}

int32_t pi_hyper_open(struct pi_device *device)
{
    struct pi_hyper_conf *conf = (struct pi_hyper_conf *) device->config;
    /* Init Hyper periph. */
    if (!hyper_periph_init)
    {
        g_hyper_device = device;
        /* Init event list. */
        struct hyper_driver_fifo *fifo = (struct hyper_driver_fifo *) pmsis_l2_malloc(sizeof(struct hyper_driver_fifo));
        if (fifo == NULL)
        {
            return -1;
        }
        fifo->fifo_head = NULL;
        fifo->fifo_tail = NULL;
        __global_hyper_driver_fifo[conf->id] = fifo;

        /* Set handlers. */
        pi_fc_event_handler_set(SOC_EVENT_UDMA_HYPER_RX(conf->id), hyper_handler);
        pi_fc_event_handler_set(SOC_EVENT_UDMA_HYPER_TX(conf->id), hyper_handler);
        /* Enable SOC events propagation to FC. */
        hal_soc_eu_set_fc_mask(SOC_EVENT_UDMA_HYPER_RX(conf->id));
        hal_soc_eu_set_fc_mask(SOC_EVENT_UDMA_HYPER_TX(conf->id));

        /* Pin settings. */
        __pi_hyper_pin_settings(13, HYPERBUS_DQ0, HYPERBUS_DQ1, HYPERBUS_DQ2,
                                HYPERBUS_DQ3, HYPERBUS_DQ4, HYPERBUS_DQ5,
                                HYPERBUS_DQ6, HYPERBUS_DQ7, HYPERBUS_CLK,
                                HYPERBUS_CLKN, HYPERBUS_RWDS, HYPERBUS_CSN0, HYPERBUS_CSN1);
        /* Disable UDMA CG. */
        udma_init_device(UDMA_HYPER_ID(conf->id));

        /* Hyper config. */
        __pi_hyper_settings(conf->cs);

        hyper_periph_init = 1;
    }
    /* Init Hyperram memory allocator. */
    if (conf->type == PI_HYPER_TYPE_RAM)
    {
        if (hyper_ram_init)
        {
            return -1;
        }
        /* Init external malloc. */
        malloc_t *ext_malloc = (malloc_t *) pmsis_l2_malloc(sizeof(malloc_t));
        if (ext_malloc == NULL)
        {
            return -1;
        }
        if (__pi_hyper_alloc_init(ext_malloc, 0, conf->ram_size))
        {
            return -1;
        }
        device->data = (void *) ext_malloc;

        hyper_ram_init = 1;
    }
    /* Init Hyperflash. */
    if (conf->type == PI_HYPER_TYPE_FLASH)
    {
        if (hyper_flash_init)
        {
            return -1;
        }
        g_hyper_device = device;
        /* Init the flash. */
        __pi_hyper_flash_config(device);

        hyper_flash_init = 1;
    }
    return 0;
}

void pi_hyper_close(struct pi_device *device)
{
    if (!hyper_periph_init && !hyper_flash_init && !hyper_ram_init)
    {
        struct pi_hyper_conf *conf = (struct pi_hyper_conf *) device->config;
        /* Clear handlers. */
        pi_fc_event_handler_clear(SOC_EVENT_UDMA_HYPER_RX(conf->id));
        pi_fc_event_handler_clear(SOC_EVENT_UDMA_HYPER_TX(conf->id));
        /* Disable SOC events propagation to FC. */
        hal_soc_eu_clear_fc_mask(SOC_EVENT_UDMA_HYPER_RX(conf->id));
        hal_soc_eu_clear_fc_mask(SOC_EVENT_UDMA_HYPER_TX(conf->id));

        /* Enable UDMA CG. */
        udma_deinit_device(UDMA_HYPER_ID(conf->id));

        /* Free external malloc struct. */
        __pi_hyper_alloc_deinit((malloc_t *) device->data);
    }
}

void pi_hyper_read(struct pi_device *device, uint32_t hyper_addr,
                   void *buffer, uint32_t size)
{
    pi_task_t task_block;
    pi_task_block(&task_block);

    pi_hyper_read_async(device, hyper_addr, buffer, size, &task_block);

    pi_task_wait_on(&task_block);
    pi_task_destroy(&task_block);
}

void pi_hyper_read_async(struct pi_device *device, uint32_t hyper_addr,
                         void *buffer, uint32_t size, struct pi_task *callback)
{
    struct pi_hyper_conf *conf = device->config;
    uint32_t ext_addr = hyper_addr;
    ext_addr += (conf->type == PI_HYPER_TYPE_FLASH) ? (uint32_t) REG_MBR1 : 0;
    struct hyper_transfer_s transfer = {0};
    transfer.hyper_addr = ext_addr;
    transfer.buffer = buffer;
    transfer.size = size;
    transfer.stride = 0;
    transfer.channel = RX_CHANNEL;
    transfer.device_id = conf->id;
    DEBUG_PRINTF("Transfer : %x %x %d\n", transfer.hyper_addr, transfer.buffer, transfer.size);
    __pi_hyper_copy(transfer.device_id, &transfer, callback);
}

void pi_hyper_write(struct pi_device *device, uint32_t hyper_addr,
                    void *buffer, uint32_t size)
{
    pi_task_t task_block;
    pi_task_block(&task_block);

    pi_hyper_write_async(device, hyper_addr, buffer, size, &task_block);

    pi_task_wait_on(&task_block);
    pi_task_destroy(&task_block);
}

void pi_hyper_write_async(struct pi_device *device, uint32_t hyper_addr,
                          void *buffer, uint32_t size, struct pi_task *callback)
{
    struct pi_hyper_conf *conf = (struct pi_hyper_conf *)device->config;
    uint32_t ext_addr = hyper_addr;
    ext_addr += (conf->type == PI_HYPER_TYPE_FLASH) ? (uint32_t) REG_MBR1 : 0;
    struct hyper_transfer_s transfer = {0};
    transfer.hyper_addr = ext_addr;
    transfer.buffer = buffer;
    transfer.size = size;
    transfer.stride = 0;
    transfer.channel = TX_CHANNEL;
    transfer.device_id = conf->id;
    DEBUG_PRINTF("Transfer : %x %x %d %x\n", transfer.hyper_addr, transfer.buffer, transfer.size, conf->type);
    __pi_hyper_copy(transfer.device_id, &transfer, callback);
}

void pi_hyper_read_2d(struct pi_device *device, uint32_t hyper_addr,
                      void *buffer, uint32_t size,
                      uint32_t stride, uint32_t length)
{
    pi_task_t task_block;
    pi_task_block(&task_block);

    pi_hyper_read_2d_async(device, hyper_addr, buffer, size, stride, length, &task_block);

    pi_task_wait_on(&task_block);
    pi_task_destroy(&task_block);
}

void pi_hyper_read_2d_async(struct pi_device *device, uint32_t hyper_addr,
                            void *buffer, uint32_t size,
                            uint32_t stride, uint32_t length, struct pi_task *callback)
{
    struct pi_hyper_conf *conf = (struct pi_hyper_conf *) device->config;
    uint32_t ext_addr = hyper_addr;
    ext_addr += (conf->type == PI_HYPER_TYPE_FLASH) ? (uint32_t) REG_MBR1 : 0;
    struct hyper_transfer_s transfer = {0};
    transfer.hyper_addr = ext_addr;
    transfer.buffer = buffer;
    transfer.size = size;
    transfer.stride = stride;
    transfer.length = length;
    transfer.channel = RX_CHANNEL;
    transfer.device_id = conf->id;
    DEBUG_PRINTF("Transfer : %x %x %d %x\n", transfer.hyper_addr, transfer.buffer, transfer.size, conf->type);
    __pi_hyper_copy_2d(conf->id, &transfer, callback);
}

void pi_hyper_write_2d(struct pi_device *device, uint32_t hyper_addr,
                       void *buffer, uint32_t size,
                       uint32_t stride, uint32_t length)
{
    pi_task_t task_block;
    pi_task_block(&task_block);

    pi_hyper_write_2d_async(device, hyper_addr, buffer, size, stride, length, &task_block);

    pi_task_wait_on(&task_block);
    pi_task_destroy(&task_block);
}

void pi_hyper_write_2d_async(struct pi_device *device, uint32_t hyper_addr,
                             void *buffer, uint32_t size,
                             uint32_t stride, uint32_t length, struct pi_task *callback)
{
    struct pi_hyper_conf *conf = device->config;
    uint32_t ext_addr = hyper_addr;
    ext_addr += (conf->type == PI_HYPER_TYPE_FLASH) ? (uint32_t) REG_MBR1 : 0;
    struct hyper_transfer_s transfer = {0};
    transfer.hyper_addr = ext_addr;
    transfer.buffer = buffer;
    transfer.size = size;
    transfer.stride = stride;
    transfer.length = length;
    transfer.channel = TX_CHANNEL;
    transfer.device_id = conf->id;
    DEBUG_PRINTF("Transfer : %x %x %d\n", transfer.hyper_addr, transfer.buffer, transfer.size);
    __pi_hyper_copy_2d(conf->id, &transfer, callback);
}

uint32_t pi_hyperram_alloc(struct pi_device *device, uint32_t size)
{
    return (uint32_t) __malloc_extern((malloc_t *) device->data, size);
}

int32_t pi_hyperram_free(struct pi_device *device, uint32_t chunk, uint32_t size)
{
    return __malloc_extern_free((malloc_t *) device->data, (void *) chunk, size);
}

#if (FEATURE_CLUSTER ==1)

static void __pi_cl_delegate_hyper_copy(void *arg)
{
    pi_cl_hyper_req_t *req = (pi_cl_hyper_req_t *) arg;
    struct pi_hyper_conf *conf = req->device->config;
    if (req->is_2d)
    {
        __pi_hyper_copy_2d(conf->id, &(req->transfer), &(req->task_done));
    }
    else
    {
        __pi_hyper_copy(conf->id, &(req->transfer), &(req->task_done));
    }
}

static void __pi_cl_hyper_copy(struct pi_device *device, uint32_t hyper_addr, void *buffer,
                               uint32_t size, udma_channel_e read, pi_cl_hyper_req_t *req)
{
    struct pi_hyper_conf *conf = device->config;
    uint32_t ext_addr = hyper_addr;
    ext_addr += (conf->type == PI_HYPER_TYPE_FLASH) ? (uint32_t) REG_MBR1 : 0;
    /* Transfer struct. */
    req->transfer.hyper_addr = ext_addr;
    req->transfer.buffer = buffer;
    req->transfer.size = size;
    req->transfer.stride = 0;
    req->transfer.length = 0;
    req->transfer.channel = read;
    req->transfer.device_id = conf->id;
    req->device = device;
    req->cid = pi_cluster_id();
    req->is_2d = 0;
    /* Callback. */
    pi_task_callback_no_mutex(&(req->cb), __pi_cl_delegate_hyper_copy, req);
    cl_send_task_to_fc(&(req->cb));
}

static void __pi_cl_hyper_copy_2d(struct pi_device *device, uint32_t hyper_addr,
                                  void *buffer, uint32_t size, uint32_t stride,
                                  uint32_t length, udma_channel_e read, pi_cl_hyper_req_t *req)
{
    struct pi_hyper_conf *conf = device->config;
    uint32_t ext_addr = hyper_addr;
    ext_addr += (conf->type == PI_HYPER_TYPE_FLASH) ? (uint32_t) REG_MBR1 : 0;
    /* Transfer struct. */
    req->transfer.hyper_addr = ext_addr;
    req->transfer.buffer = buffer;
    req->transfer.size = size;
    req->transfer.stride = stride;
    req->transfer.length = length;
    req->transfer.channel = read;
    req->transfer.device_id = conf->id;
    req->device = device;
    req->cid = pi_cluster_id();
    req->is_2d = 1;
    /* Callback. */
    pi_task_callback_no_mutex(&(req->cb), __pi_cl_delegate_hyper_copy, req);
    cl_send_task_to_fc(&(req->cb));
}

void pi_cl_hyper_read(struct pi_device *device, uint32_t hyper_addr,
                                    void *addr, uint32_t size, pi_cl_hyper_req_t *req)
{
    pi_task_block_no_mutex(&(req->task_done));
    __pi_cl_hyper_copy(device, hyper_addr, addr, size, RX_CHANNEL, req);
    hal_compiler_barrier();
}

void pi_cl_hyper_read_2d(struct pi_device *device, uint32_t hyper_addr,
                                       void *addr, uint32_t size,
                                       uint32_t stride, uint32_t length, pi_cl_hyper_req_t *req)
{
    pi_task_block_no_mutex(&(req->task_done));
    __pi_cl_hyper_copy_2d(device, hyper_addr, addr, size, stride, length, RX_CHANNEL, req);
    hal_compiler_barrier();
}

void pi_cl_hyper_read_wait(pi_cl_hyper_req_t *req)
{
    pi_task_wait_on_no_mutex(&(req->task_done));
    hal_compiler_barrier();
}

void pi_cl_hyper_write(struct pi_device *device, uint32_t hyper_addr,
                                     void *addr, uint32_t size, pi_cl_hyper_req_t *req)
{
    pi_task_block_no_mutex(&(req->task_done));
    __pi_cl_hyper_copy(device, hyper_addr, addr, size, TX_CHANNEL, req);
    hal_compiler_barrier();
}

void pi_cl_hyper_write_2d(struct pi_device *device, uint32_t hyper_addr,
                                        void *addr, uint32_t size,
                                        uint32_t stride, uint32_t length, pi_cl_hyper_req_t *req)
{
    pi_task_block_no_mutex(&(req->task_done));
    __pi_cl_hyper_copy_2d(device, hyper_addr, addr, size, stride, length, TX_CHANNEL, req);
    hal_compiler_barrier();
}

void pi_cl_hyper_write_wait(pi_cl_hyper_req_t *req)
{
    pi_task_wait_on_no_mutex(&(req->task_done));
    hal_compiler_barrier();
}


static void __pi_cl_delegate_hyper_alloc(void *arg)
{
    pi_cl_hyper_alloc_req_t *req = (pi_cl_hyper_alloc_req_t *) arg;
    req->result = pi_hyperram_alloc(req->device, req->size);
}

void pi_cl_hyperram_alloc(struct pi_device *device, uint32_t size,
                          pi_cl_hyper_alloc_req_t *req)
{
    req->device = device;
    req->size = size;
    req->cid = pi_cluster_id();
    /* Callback. */
    pi_task_callback_no_mutex(&(req->cb), __pi_cl_delegate_hyper_alloc, req);
    cl_send_task_to_fc(&(req->cb));
    hal_compiler_barrier();
}

static void __pi_cl_delegate_hyper_free(void *arg)
{
    pi_cl_hyper_free_req_t *req = (pi_cl_hyper_free_req_t *) arg;
    req->result = pi_hyperram_free(req->device, req->chunk, req->size);
}

void pi_cl_hyperram_free(struct pi_device *device, uint32_t chunk, uint32_t size,
                         pi_cl_hyper_free_req_t *req)
{
    req->device = device;
    req->size = size;
    req->chunk = chunk;
    req->cid = pi_cluster_id();
    /* Callback. */
    pi_task_callback_no_mutex(&(req->cb), __pi_cl_delegate_hyper_free, req);
    cl_send_task_to_fc(&(req->cb));
    hal_compiler_barrier();
}

uint32_t pi_cl_hyperram_alloc_wait(pi_cl_hyper_alloc_req_t *req)
{
    pi_task_wait_on_no_mutex(&(req->cb));
    hal_compiler_barrier();
    return req->result;
}

void pi_cl_hyperram_free_wait(pi_cl_hyper_free_req_t *req)
{
    pi_task_wait_on_no_mutex(&(req->cb));
    hal_compiler_barrier();
}

#endif  /* (FEATURE_CLUSTER == 1) */

static int32_t __pi_hyper_resume_misaligned_write(struct pi_task *task)
{
    while(1)
    {
        // Compute information to see how to do one more step
        int32_t addr_aligned = (pending_emu_buffer + 3) & ~0x3;
        int32_t prologue_size = addr_aligned - pending_emu_buffer;
        int32_t hyper_addr_aligned = pending_emu_hyper_addr + prologue_size;

        if (pending_emu_size < 4)
        {
            prologue_size = pending_emu_size;
        }

        if (prologue_size)
        {
            // Case where we have a partial copy to do
            if (!pending_emu_do_memcpy)
            {
                /*
                 * A partial transfer must first transfer the content of the hyper
                 * to the temporary area and partially overwrite it with a memcpy.
                 * This part is first called to trigger the transfer while the part after
                 * is called to do the memcpy and the final transfer as a second step.
                 */
                struct hyper_transfer_s transfer = {0};
                transfer.channel = RX_CHANNEL;
                transfer.device_id = pending_emu_device_id;
                transfer.hyper_addr = pending_emu_hyper_addr & ~0x1;
                transfer.buffer = __pi_hyper_temp_buffer;
                transfer.size = 4;
                __pi_hyper_copy_aligned( &transfer, NULL);

                // It is asynchronous, just remember we have to do
                // a memcpy when the transfer is done and leave
                pending_emu_do_memcpy = 1;
                return 0;
            }

            pending_emu_do_memcpy = 0;
            l2_memcpy(&__pi_hyper_temp_buffer[pending_emu_hyper_addr & 0x1],
                      (void *) pending_emu_buffer,
                      prologue_size);

            struct hyper_transfer_s transfer = {0};
            transfer.channel = TX_CHANNEL;
            transfer.device_id = pending_emu_device_id;
            transfer.hyper_addr = pending_emu_hyper_addr & ~0x1;
            transfer.buffer = __pi_hyper_temp_buffer;
            transfer.size = 4;
            __pi_hyper_copy_aligned( &transfer, NULL);

            pending_emu_hyper_addr += prologue_size;
            pending_emu_buffer += prologue_size;
            pending_emu_size -= prologue_size;

            return 0;
        }
        else if (pending_emu_size > 0)
        {
            // Case where we have the body to transfer
            uint32_t size_aligned = pending_emu_size & ~0x3;

            if ((hyper_addr_aligned & 0x1) == 0)
            {
                // Good case where the body is aligned on both sides and we can do
                // a direct copy.
                struct hyper_transfer_s transfer = {0};
                transfer.channel = pending_emu_channel;
                transfer.device_id = pending_emu_device_id;
                transfer.hyper_addr = pending_emu_hyper_addr;
                transfer.buffer = (void *) pending_emu_buffer;
                transfer.size = size_aligned;
                __pi_hyper_copy_aligned( &transfer, NULL);

                pending_emu_hyper_addr += size_aligned;
                pending_emu_buffer += size_aligned;
                pending_emu_size -= size_aligned;

                // It is asynchronous, just leave, we'll continue the transfer
                // when this one is over
                return 0;
            }
            else
            {
                // Bad case where we have to transfer the body using a temporary
                // buffer as the aligments on both sides are not compatible.
                // This part is very similar to the prologue.
                // Just be careful to split into small transfers to fit the temporary buffer.
                if (size_aligned > __PI_HYPER_TEMP_BUFFER_SIZE - 4)
                {
                    size_aligned = __PI_HYPER_TEMP_BUFFER_SIZE - 4;
                }

                if (!pending_emu_do_memcpy)
                {
                    struct hyper_transfer_s transfer = {0};
                    transfer.channel = RX_CHANNEL;
                    transfer.device_id = pending_emu_device_id;
                    transfer.hyper_addr = pending_emu_hyper_addr & ~0x1;
                    transfer.buffer = __pi_hyper_temp_buffer;
                    transfer.size = 4;
                    __pi_hyper_copy_aligned( &transfer, NULL);

                    pending_emu_do_memcpy = 1;
                    return 0;
                }

                pending_emu_do_memcpy = 0;
                l2_memcpy(&__pi_hyper_temp_buffer[1],
                          (void *) pending_emu_buffer,
                          (size_aligned - 1));

                struct hyper_transfer_s transfer = {0};
                transfer.channel = pending_emu_channel;
                transfer.device_id = pending_emu_device_id;
                transfer.hyper_addr = pending_emu_hyper_addr & ~0x1;
                transfer.buffer = __pi_hyper_temp_buffer;
                transfer.size = size_aligned;
                __pi_hyper_copy_aligned( &transfer, NULL);

                pending_emu_hyper_addr += size_aligned-1;
                pending_emu_buffer += size_aligned-1;
                pending_emu_size -= size_aligned-1;

                return 0;
            }
        }
        // Now check if we are done
        if (pending_emu_size == 0)
        {
            // Check if we are doing a 2D transfer
            if (pending_emu_size_2d > 0)
            {
                // In this case, update the global size
                if (pending_emu_size_2d > pending_emu_length)
                {
                    pending_emu_size_2d -= pending_emu_length;
                }
                else
                {
                    pending_emu_size_2d = 0;
                }

                // And check if we must reenqueue a line.
                if (pending_emu_size_2d > 0)
                {
                    pending_emu_hyper_addr = pending_emu_hyper_addr - pending_emu_length + pending_emu_stride;
                    pending_emu_size = (pending_emu_size_2d > pending_emu_length) ? pending_emu_length : pending_emu_size_2d;
                    continue;
                }
            }

            pending_emu_task = NULL;
            __pi_hyper_handle_end_of_task(task);

            return 1;
        }
        break;
    }
    return 0;
}

static int32_t __pi_hyper_resume_misaligned_read(struct pi_task *task)
{
    while (1)
    {
        // Compute information to see how to do one more step
        int32_t addr_aligned = (pending_emu_buffer + 3) & ~0x3;
        int32_t prologue_size = addr_aligned - pending_emu_buffer;
        int32_t hyper_addr_aligned = pending_emu_hyper_addr + prologue_size;

        if (pending_emu_size < 4)
        {
            prologue_size = pending_emu_size;
        }

        if (prologue_size)
        {
            // Case where we have a partial copy to do
            if (!pending_emu_do_memcpy)
            {
                /*
                 * A partial transfer must first transfer to the temporary area
                 * and finish the transfer by hands using a memcpy.
                 * This part is first called to trigger the transfer while the part after
                 * is called to do the memcpy as a second step.
                 */
                struct hyper_transfer_s transfer = {0};
                transfer.channel = RX_CHANNEL;
                transfer.device_id = pending_emu_device_id;
                transfer.hyper_addr = pending_emu_hyper_addr & ~0x1;
                transfer.buffer = __pi_hyper_temp_buffer;
                transfer.size = 4;
                __pi_hyper_copy_aligned( &transfer, NULL);

                // It is asynchronous, just remember we have to do
                // a memcpy when the transfer is done and leave
                pending_emu_do_memcpy = 1;
                return 0;
            }

            pending_emu_do_memcpy = 0;
            l2_memcpy((void *)pending_emu_buffer,
                      &__pi_hyper_temp_buffer[pending_emu_hyper_addr & 0x1],
                      prologue_size);

            pending_emu_hyper_addr += prologue_size;
            pending_emu_buffer += prologue_size;
            pending_emu_size -= prologue_size;

            // The transfer is asynchronous, we get there to do the memcpy
            // without triggering any transfer, so we can start again to trigger one.
            if (pending_emu_size)
            {
                continue;
            }
        }
        else if (pending_emu_size > 0)
        {
            // Case where we have the body to transfer
            uint32_t size_aligned = pending_emu_size & ~0x3;

            if ((hyper_addr_aligned & 0x1) == 0)
            {
                // Good case where the body is aligned on both sides and we can do
                // a direct copy.
                struct hyper_transfer_s transfer = {0};
                transfer.channel = pending_emu_channel;
                transfer.device_id = pending_emu_device_id;
                transfer.hyper_addr = pending_emu_hyper_addr;
                transfer.buffer = (void *) pending_emu_buffer;
                transfer.size = size_aligned;
                __pi_hyper_copy_aligned( &transfer, NULL);

                pending_emu_hyper_addr += size_aligned;
                pending_emu_buffer += size_aligned;
                pending_emu_size -= size_aligned;

                // It is asynchronous, just leave, we'll continue the transfer
                // when this one is over
                return 0;
            }
            else
            {
                // Bad case where we have to transfer the body using a temporary
                // buffer as the aligments on both sides are not compatible.
                // This part is very similar to the prologue.
                // Just be careful to split into small transfers to fit the temporary buffer.
                if (size_aligned > __PI_HYPER_TEMP_BUFFER_SIZE - 4)
                {
                    size_aligned = __PI_HYPER_TEMP_BUFFER_SIZE - 4;
                }

                if (!pending_emu_do_memcpy)
                {
                    struct hyper_transfer_s transfer = {0};
                    transfer.channel = pending_emu_channel;
                    transfer.device_id = pending_emu_device_id;
                    transfer.hyper_addr = pending_emu_hyper_addr & ~0x1;
                    transfer.buffer = __pi_hyper_temp_buffer;
                    transfer.size = size_aligned + 4;
                    __pi_hyper_copy_aligned( &transfer, NULL);

                    pending_emu_do_memcpy = 1;
                    return 0;
                }

                pending_emu_do_memcpy = 0;
                l2_memcpy((void *) pending_emu_buffer,
                          &__pi_hyper_temp_buffer[1],
                          size_aligned);

                pending_emu_hyper_addr += size_aligned;
                pending_emu_buffer += size_aligned;
                pending_emu_size -= size_aligned;

                if (pending_emu_size)
                {
                    continue;
                }
            }
        }
        // Now check if we are done
        if (pending_emu_size == 0)
        {
            // Check if we are doing a 2D transfer
            if (pending_emu_size_2d > 0)
            {
                // In this case, update the global size
                if (pending_emu_size_2d > pending_emu_length)
                {
                    pending_emu_size_2d -= pending_emu_length;
                }
                else
                {
                    pending_emu_size_2d = 0;
                }

                // And check if we must reenqueue a line.
                if (pending_emu_size_2d > 0)
                {
                    pending_emu_hyper_addr = pending_emu_hyper_addr - pending_emu_length + pending_emu_stride;
                    pending_emu_size = (pending_emu_size_2d > pending_emu_length) ? pending_emu_length : pending_emu_size_2d;
                    continue;
                }
            }

            pending_emu_task = NULL;
            __pi_hyper_handle_end_of_task(task);

            return 1;
        }
        break;
    }
    return 0;
}


static void __pi_hyper_copy_misaligned(struct pi_task *task)
{
    uint32_t end = 0;
    if (pending_emu_channel == TX_CHANNEL)
    {
        end = __pi_hyper_resume_misaligned_write(task);
    }
    else
    {
        end = __pi_hyper_resume_misaligned_read(task);
    }
    if (!end)
    {
        return;
    }
    exec_pending_task();
}

static void __pi_hyper_resume_emu_task()
{
    __pi_hyper_copy_misaligned(pending_emu_task);
}
