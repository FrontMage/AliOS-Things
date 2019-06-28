/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>

#include <hal/base.h>
#include <aos/kernel.h>
#include <k_api.h>

ktask_t *g_goodbye_thread;

static void goodbye_thread(void)
{
    int count = 0;
    printf("Second nano entry here!\r\n");
    while(1) {
        printf("Goodbye world! count %d \r\n", count++);
#ifdef TEST_SYSTICKS
        for(volatile int i = 0; i< 100000; i++);
#else
        aos_msleep(10);
#endif
    };
}

int application_start(int argc, char *argv[])
{
    printf("nano entry here!\r\n");

    uint32_t *malloc_test = aos_malloc(sizeof(uint32_t));
    printf("malloc_test=%p\n", malloc_test);
    int count = 0;
    uint32_t task_stack_size = 256;
    int ret = krhino_task_dyn_create(&g_goodbye_thread, "goodbye thread", NULL, AOS_DEFAULT_APP_PRI, 0, task_stack_size, goodbye_thread, 1);

    while(1) {
        printf("hello world! count %d \r\n", count++);
#ifdef TEST_SYSTICKS
        for(volatile int i = 0; i< 100000; i++);
#else
        aos_msleep(10);
#endif
    };
}
