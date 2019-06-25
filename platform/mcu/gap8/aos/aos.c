/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <k_api.h>
#include <hal/base.h>
#include <aos/kernel.h>
#include <aos/hal/uart.h>
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "cores/TARGET_RISCV_32/pmsis_gcc.h"

#define AOS_START_STACK 512

extern int application_start(int argc, char **argv);
extern int vfs_init(void);
extern int vfs_device_init(void);
extern uart_dev_t uart_0;

void __systick_handler(void);

ktask_t *g_aos_app;
static void sys_init(void)
{
#if 0 // to be on the safe side!
#ifdef AOS_VFS
    vfs_init();
    vfs_device_init();
#endif

#ifdef CONFIG_AOS_CLI
    aos_cli_init();
#endif

#ifdef WITH_SAL
    sal_device_init();
#endif

// port to PMSIS on top?
#ifdef AOS_LOOP
    aos_loop_init();
#endif
#endif

    system_setup_systick(RHINO_CONFIG_TICKS_PER_SECOND);
    // client application start
    __enable_irq();
    application_start(0, NULL);
}

static void platform_init(void)
{
#ifdef __USE_DEBUG_CONSOLE__
    board_init_debug_console();
#endif
}

#define us2tick(us) \
    ((us * RHINO_CONFIG_TICKS_PER_SECOND + 999999) / 1000000)

void hal_reboot(void)
{
    event_system_reset();
}


void __systick_handler(void)
{
    krhino_intrpt_enter();
    krhino_tick_proc();
    krhino_intrpt_exit();
}

int main(void)
{
    platform_init();

    hal_uart_init(&uart_0);
    aos_init();
    krhino_task_dyn_create(&g_aos_app, "aos-init", 0, AOS_DEFAULT_APP_PRI, 0, AOS_START_STACK, (task_entry_t)sys_init, 1);

    // uses first_task_start
    aos_start();

    return 0;
}
