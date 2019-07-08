/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <assert.h>
//#include <stdio.h>	
#include "soc_impl.h"
#if 0
#include "fsl_device_registers.h"

#if (RHINO_CONFIG_HW_COUNT > 0)
void soc_hw_timer_init(void)
{
}

hr_timer_t soc_hr_hw_cnt_get(void)
{
    return 0;
}

lr_timer_t soc_lr_hw_cnt_get(void)
{
    return 0;
}
#endif /* RHINO_CONFIG_HW_COUNT */

#if (RHINO_CONFIG_INTRPT_GUARD > 0)
void soc_intrpt_guard(void)
{
}
#endif

#if (RHINO_CONFIG_INTRPT_STACK_REMAIN_GET > 0)
size_t soc_intrpt_stack_remain_get(void)
{
    return 0;
}
#endif

#if (RHINO_CONFIG_INTRPT_STACK_OVF_CHECK > 0)
void soc_intrpt_stack_ovf_check(void)
{
}
#endif

#if (RHINO_CONFIG_DYNTICKLESS > 0)
void soc_tick_interrupt_set(tick_t next_ticks,tick_t elapsed_ticks)
{
}

tick_t soc_elapsed_ticks_get(void)
{
    return 0;
}
#endif
#endif

size_t soc_get_cur_sp()
{
    size_t sp = 0;
#if defined (__GNUC__)
	__asm volatile("mv %0,sp\n" :"=r"(sp));
#endif
    return sp;
}

static void soc_print_stack()
{
    void    *cur, *end;
    int      i=0;
    int     *p;

    end   = krhino_cur_task_get()->task_stack_base + krhino_cur_task_get()->stack_size;
    cur = (void *)soc_get_cur_sp();
    p = (int*)cur;
    while(p < (int*)end) {
        if(i%4==0) {
            printf("\r\n%08x:",(uint32_t)p);
        }
        printf("%08x ", *p);
        i++;
        p++;
    }
    printf("\r\n");
    return;
}

#include <aos/hal/uart.h>
extern uart_dev_t uart_0;

void soc_err_proc(kstat_t err)
{
    (void)err;
    soc_print_stack();
    assert(0);
}

krhino_err_proc_t g_err_proc = soc_err_proc;

#if defined (__GNUC__)

extern char __heapl2ram_start;
extern char __heapl2ram_size;

k_mm_region_t g_mm_region[] =
{
    {(uint8_t*)&__heapl2ram_start, (uint32_t)&__heapl2ram_size},
};

#else
#error "Tool chain not supported!"
#endif

#define PRINTF_USE_UART

#include "tinyprintf.h"
#include "gap_debug.h"
#define PRINTF_BUFF_SIZE 16

uint32_t g_printf_buff_cur_size = 0;
char g_printf_buff[PRINTF_BUFF_SIZE];

void tfp_putc(void *data, char c)
{
#ifdef PRINTF_USE_UART
    g_printf_buff[g_printf_buff_cur_size] = c;
    g_printf_buff_cur_size++;
    if((c=='\n') || (g_printf_buff_cur_size==PRINTF_BUFF_SIZE))
    {
        hal_uart_send(&uart_0, g_printf_buff, g_printf_buff_cur_size, -1);

        g_printf_buff_cur_size = 0;
    }
#else
    // Iter until we can push the character.
    while (DEBUG_PutcharNoPoll(DEBUG_GetDebugStruct(), c))
    {
    }

    // If the buffer has been flushed to the bridge, we now need to send him
    // a notification
    if (DEBUG_IsEmpty(DEBUG_GetDebugStruct()))
    {
        BRIDGE_PrintfFlush();
    }
#endif
}

static kmutex_t g_printf_mutex;
static int printf_is_init = 0;

// bridge to the tinyprintf
__attribute__ ((export))
int printf(const char *fmt, ...)
{
    if(!printf_is_init)
    {
        krhino_mutex_create(&g_printf_mutex, "g_printf_mutex");
        krhino_mutex_unlock(&g_printf_mutex);
#ifndef PRINTF_USE_UART
        BRIDGE_Init();
#endif
        printf_is_init= 1;
    }
    krhino_mutex_lock(&g_printf_mutex, RHINO_WAIT_FOREVER);
    va_list va;
    va_start(va, fmt);
    /* Only lock the printf if the cluster is up to avoid mixing FC and cluster output */
    // ideally should lock here
    tfp_format(NULL, tfp_putc, fmt, va);
    // ideally should unlock here
    va_end(va);
    krhino_mutex_unlock(&g_printf_mutex);
    return 0;
}

__attribute__ ((export))
int puts(const char *str)
{
    char c;
    do {
        c = *str;
        if (c == 0) {
            tfp_putc(NULL, '\n');
            break;
        }
        tfp_putc(NULL, c);
        str++;
    } while(1);
    return 0;
}

int g_region_num = sizeof(g_mm_region)/sizeof(k_mm_region_t);
