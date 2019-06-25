/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>

#include <aos/kernel.h>

#include "rtos/pmsis_driver_core_api/pmsis_driver_core_api.h"
#include "rtos/os_frontend_api/pmsis_task.h"
#include "pmsis_cluster/cluster_sync/fc_to_cl_delegate.h"
#include "pmsis_cluster/cluster_team/cl_team.h"

#include "cores/TARGET_RISCV_32/core_utils.h"
// to make a pseudo delegation
char cluster_string[] = "Cluster says: hello there\n\0";
volatile uint8_t string_ready = 0;
volatile uint8_t cl_count[8] = {0};

void cluster_callback(void *arg)
{
    printf("Cluster callback is executing\n");
}

void cluster_slave_work(void *arg)
{
    (*(volatile uint8_t*)&cl_count[__native_core_id()])++;
}

void cluster_master_entry(void *arg)
{
    string_ready = 1;
    cl_team_fork(8, cluster_slave_work, arg);
}

int application_start(int argc, char *argv[])
{
    int count = 0;
    printf("nano entry here!\r\n");
#if 1
    struct pi_device *cluster_dev = aos_malloc(sizeof(struct pi_device));
    struct cluster_driver_conf *conf = aos_malloc(sizeof(struct cluster_driver_conf));
    
    conf->device_type = 0;
    conf->id = 0;

    pi_open_from_conf(cluster_dev, conf);
    printf("Cluster is open\n");
#if 1
    struct pi_cluster_task *task = aos_malloc(sizeof(struct pi_cluster_task));
    memset(task, 0, sizeof(struct pi_cluster_task));
    task->entry = cluster_master_entry;
    task->arg = NULL;

    pi_task_t callback_task;
    pi_task_callback(&callback_task, cluster_callback, NULL);
    pi_cluster_send_task_to_cl_async(cluster_dev, task, &callback_task);
#endif
#endif
    while(1) {
        printf("hello world! count %d / string ready=%d\n", count++, string_ready);

        for(int i=0; i<8; i++)
        {
            printf("cl_count[%i]=%d\n",i,cl_count[i]);
        }

        if(string_ready)
        {
            printf("%s",cluster_string);
        }

        aos_msleep(1000);
    };
}
