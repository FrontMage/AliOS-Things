/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <k_api.h>
#include <k_mm.h>
#include <hal/base.h>
#include <aos/kernel.h>
#include <aos/hal/uart.h>
#include "aos/init.h"
#include "aos/cli.h"
#include "aos/yloop.h"
#include "board.h"
#include "cores/TARGET_RISCV_32/pmsis_gcc.h"
#include "pmsis.h"
#include "pmsis/rtos/event_kernel/event_kernel.h"

#define AOS_START_STACK 512

extern int application_start(int argc, char **argv);
extern int vfs_init(void);
extern int vfs_device_init(void);
static void platform_init(void);
extern uart_dev_t uart_0;
extern aos_loop_t aos_loop_init(void);
extern void ulog_init(void);
extern int32_t kv_init(void);

void __systick_handler(void);

ktask_t *g_aos_app;


extern char __heapfcram_start;
extern char __heapfcram_size;

extern char __heapl2osram_start;
extern char __heapl2osram_size;

static void sys_init(void)
{
    system_init();
    // We are now ready to handle systicks
    system_setup_systick(RHINO_CONFIG_TICKS_PER_SECOND);
    // prepare default event kernel/workqueue for pmsis drivers
    struct pmsis_event_kernel_wrap *wrap;
    pmsis_event_kernel_init(&wrap, pmsis_event_kernel_main);
    pmsis_event_set_default_scheduler(wrap);

#ifdef PRINTF_USE_UART
    // depends on event kernel, comes last
    hal_uart_init(&uart_0);
#endif

    platform_init();

    printf("Welcome on AliOS-Things with GAP8\n");

#ifndef NO_VFS
    vfs_init();
#endif

#ifndef CLI_DISABLED
    aos_cli_init();
#endif

#ifndef NO_VFS
    vfs_device_init();
#endif

#ifndef YLOOP_DISABLED
    aos_loop_init();
#endif
    ulog_init();
#if (!defined(NO_FLASH_PREINIT) && !defined(KV_DISABLED))
    // might be disabled for some continuous integration tests
    kv_init();
#endif

    // client application start
    application_start(0, NULL);
}

static void platform_init(void)
{
    // Actually init uart pins where needed
    board_init_debug_console();
    board_init();
}

#define us2tick(us) \
    ((us * RHINO_CONFIG_TICKS_PER_SECOND + 999999) / 1000000)

void hal_reboot(void)
{
}

void __systick_handler(void)
{
    krhino_intrpt_enter();
    krhino_tick_proc();
    krhino_intrpt_exit();
}

int main(void)
{
    aos_init();
    krhino_task_dyn_create(&g_aos_app, "aos-init", 0, AOS_DEFAULT_APP_PRI, 0, AOS_START_STACK*2, (task_entry_t)sys_init, 1);
    // uses first_task_start
    aos_start();

    return 0;
}
