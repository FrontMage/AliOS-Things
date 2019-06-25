/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>

#include <aos/kernel.h>

#define PRINTF printf
//#define PRINTF( ... ) ((void)0)
/****************************************************************************/
/* PMSIS includes */
#include "pmsis.h"
#include "pmsis_types.h"

#include "rtos/pmsis_os.h"
#include "rtos/pmsis_driver_core_api/pmsis_driver_core_api.h"

#include "pmsis_api/include/drivers/hyperbus.h"
#include "pmsis_cluster/drivers/delegate/hyperbus/hyperbus_cl_internal.h"

#include "rtos/os_frontend_api/pmsis_task.h"
#include "pmsis_cluster/cluster_sync/fc_to_cl_delegate.h"
#include "pmsis_cluster/cluster_team/cl_team.h"
/* Variables used. */
#define BUFFER_SIZE 2048

static uint8_t *buff;
static uint8_t *rcv_buff, *rcv_buff2, *rcv_buff3;
static uint32_t hyper_buff;
struct pi_hyper_conf conf;
struct pi_device hyper;
static uint32_t done = 0;

void __end_of_tx(void *arg)
{
    pi_hyper_read(&hyper, hyper_buff, rcv_buff, BUFFER_SIZE);
}

void cluster_callback(void *arg)
{
    PRINTF("Callback is executing\n");
}

void master_entry(void *arg)
{
    //PRINTF("Master cluster entry\n");
    hal_compiler_barrier();
    pi_cl_hyperram_alloc_req_t alloc_req;
    pi_cl_hyperram_alloc(&hyper, BUFFER_SIZE, &alloc_req);
    pi_cl_hyperram_alloc_wait(&alloc_req);
    hal_compiler_barrier();
    //PRINTF("Allocated : %d %x\n", alloc_req.size, alloc_req.result);

    pi_cl_hyper_req_t buff_req, buff_req1, buff_req2;
    pi_cl_hyper_write(&hyper, alloc_req.result, buff, alloc_req.size, &buff_req);
    pi_cl_hyper_write_wait(&buff_req);
    //PRINTF("Write done\n");

    pi_cl_hyper_read(&hyper, alloc_req.result, rcv_buff2, alloc_req.size, &buff_req1);
    pi_cl_hyper_read(&hyper, alloc_req.result, rcv_buff3, alloc_req.size, &buff_req2);
    pi_cl_hyper_read_wait(&buff_req1);
    //PRINTF("Read done\n");
    pi_cl_hyper_read_wait(&buff_req2);
    //PRINTF("Read done\n");

    pi_cl_hyperram_free_req_t free_req;
    pi_cl_hyperram_free(&hyper, alloc_req.result, alloc_req.size, &free_req);
    pi_cl_hyperram_free_wait(&free_req);
    hal_compiler_barrier();
    //PRINTF("Freed : %d %x\n", free_req.size, free_req.result);
}

int application_start(int argc, char *argv[])
{
    //PRINTF("Entering main controller\n");

    buff = aos_malloc(BUFFER_SIZE);
    if (buff == NULL)
    {
        PRINTF("Tx is NULL !\n");
        return;
    }
    rcv_buff = aos_malloc(BUFFER_SIZE);
    if (rcv_buff == NULL)
    {
        PRINTF("Rx is NULL !\n");
        return;
    }
    rcv_buff2 = aos_malloc(BUFFER_SIZE);
    if (rcv_buff2 == NULL)
    {
        PRINTF("Rx is NULL !\n");
        return;
    }
    rcv_buff3 = aos_malloc(BUFFER_SIZE);
    if (rcv_buff3 == NULL)
    {
        PRINTF("Rx is NULL !\n");
        return;
    }
    for (uint32_t i=0; i<(uint32_t) BUFFER_SIZE; i++)
    {
        buff[i] = i & 0xFF;
        rcv_buff[i] = 0;
    }
    pi_hyper_conf_init(&conf);
    conf.id = 0;
    conf.ram_size = 1<<20;

    pi_open_from_conf(&hyper, &conf);

    if (pi_hyper_open(&hyper))
    {
        //PRINTF("Open failed !\n");
        return;
    }

    hyper_buff = pi_hyperram_alloc(&hyper, BUFFER_SIZE);
    if (hyper_buff == 0)
    {
        PRINTF("Hyper malloc failed !\n");
        return;
    }

    #if ASYNC
    pi_task_t callback;
    pi_task_callback(&callback, __end_of_tx, NULL);
    pi_hyper_write_async(&hyper, hyper_buff, buff, BUFFER_SIZE, &callback);
    #else
    pi_hyper_write(&hyper, hyper_buff, buff, BUFFER_SIZE);
    pi_hyper_read(&hyper, hyper_buff, rcv_buff, BUFFER_SIZE);
    #endif

    PRINTF("TX then RX done\n");

    uint32_t errors = 0;
    for (uint32_t i=0; i<(uint32_t) BUFFER_SIZE; i++)
    {
        if (buff[i] != rcv_buff[i])
        {
            errors++;
            PRINTF("%x-%x ", buff[i], rcv_buff[i]);
        }
    }

    //PRINTF("Call cluster :\n");
    struct pi_device *cluster_dev = aos_malloc(sizeof(struct pi_device));
    struct cluster_driver_conf *conf = aos_malloc(sizeof(struct cluster_driver_conf));
    conf->device_type = 0;
    conf->id = 0;
    pi_open_from_conf(cluster_dev, conf);

    //PRINTF("cluster is open :\n");
    pi_task_t *callback_task0 = pi_task_callback(aos_malloc(sizeof(pi_task_t)),
                                                 (void*)cluster_callback, (void*)NULL);
    struct pi_cluster_task *task = aos_malloc(sizeof(struct pi_cluster_task));

    memset(task, 0, sizeof(struct pi_cluster_task));
    task->entry = master_entry;
    task->arg = NULL;

    pi_cluster_send_task_to_cl_async(cluster_dev, task, callback_task0);
    pi_task_wait_on(callback_task0);
    pi_cluster_close(cluster_dev);

    for (uint32_t i=0; i<(uint32_t) BUFFER_SIZE; i++)
    {
        if (buff[i] != rcv_buff2[i])
        {
            errors++;
            PRINTF("%x-%x ", buff[i], rcv_buff2[i]);
        }
    }
    for (uint32_t i=0; i<(uint32_t) BUFFER_SIZE; i++)
    {
        if (buff[i] != rcv_buff3[i])
        {
            errors++;
            PRINTF("%x-%x ", buff[i], rcv_buff3[i]);
        }
    }

    printf("\nResult : %d error(s)\n", errors);

    aos_free(buff);
    aos_free(rcv_buff);
    aos_free(rcv_buff2);
    pi_hyperram_free(&hyper, hyper_buff, BUFFER_SIZE);
    pmsis_exit(0);
}
