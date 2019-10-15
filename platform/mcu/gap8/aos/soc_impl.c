/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <assert.h>
#include <aos/hal/uart.h>
#include "pmsis.h"
#include "soc_impl.h"
#include "tinyprintf.h"
#include "gap_debug.h"
#define PRINTF_BUFF_SIZE 16

extern uart_dev_t uart_0;
static char g_printf_buff[PRINTF_BUFF_SIZE];
static kmutex_t g_printf_mutex;
static uint32_t g_printf_buff_cur_size = 0;
static int printf_is_init = 0;
static void flush_printf_buffer(void);


static void flush_printf_buffer(void)
{
    if(g_printf_buff_cur_size)
    {
        hal_uart_send(&uart_0, g_printf_buff, g_printf_buff_cur_size, -1);
        g_printf_buff_cur_size = 0;
    }
}

void tfp_putc(void *data, char c)
{
#if defined(PRINTF_USE_UART)
    g_printf_buff[g_printf_buff_cur_size] = c;
    g_printf_buff_cur_size++;
    if((c=='\n') || (c=='\r') || (g_printf_buff_cur_size==PRINTF_BUFF_SIZE))
    {
        hal_uart_send(&uart_0, g_printf_buff, g_printf_buff_cur_size, -1);

        g_printf_buff_cur_size = 0;
    }
#elif defined(PRINTF_GVSOC)
    FC_STDOUT->PUTC[0] = c;
#else
#warning "no printf implementation"
#endif
}

// bridge to the tinyprintf
__attribute__ ((export))
int printf(const char *fmt, ...)
{
#if !defined(PRINTF_GVSOC)
    if(!pi_is_fc())
    {
        return 0;
    }
#endif
    if(!printf_is_init)
    {
        krhino_mutex_create(&g_printf_mutex, "g_printf_mutex");
        krhino_mutex_unlock(&g_printf_mutex);
        printf_is_init= 1;
    }
#if !defined(PRINTF_GVSOC)
    krhino_mutex_lock(&g_printf_mutex, RHINO_WAIT_FOREVER);
#endif
    va_list va;
    va_start(va, fmt);
    /* Only lock the printf if the cluster is up to avoid mixing FC and cluster output */
    // ideally should lock here
    tfp_format(NULL, tfp_putc, fmt, va);
    // ideally should unlock here
    va_end(va);
    flush_printf_buffer();
#if !defined(PRINTF_GVSOC)
    krhino_mutex_unlock(&g_printf_mutex);
#endif
    return 0;
}

__attribute__ ((export))
int puts(const char *str)
{
#if !defined(PRINTF_GVSOC)
    if(!pi_is_fc())
    {
        return 0;
    }
#endif
    if(!printf_is_init)
    {
        krhino_mutex_create(&g_printf_mutex, "g_printf_mutex");
        krhino_mutex_unlock(&g_printf_mutex);
        printf_is_init= 1;
    }
#if !defined(PRINTF_GVSOC)
    krhino_mutex_lock(&g_printf_mutex, RHINO_WAIT_FOREVER);
#endif
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
    flush_printf_buffer();
#if !defined(PRINTF_GVSOC)
    krhino_mutex_unlock(&g_printf_mutex);
#endif
    return 0;
}

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
            printf("\r\n%08lx:",(uint32_t)p);
        }
        printf("%08x ", *p);
        i++;
        p++;
    }
    printf("\r\n");
    return;
}

void soc_err_proc(kstat_t err)
{
    (void)err;
    soc_print_stack();
    assert(0);
}

krhino_err_proc_t g_err_proc = soc_err_proc;

#if defined (__GNUC__)

extern char __heapfcram_start;
extern char __heapfcram_size;

k_mm_region_t g_mm_region[] =
{
    {(uint8_t*)&__heapfcram_start, (uint32_t)&__heapfcram_size},
};

#else
#error "Tool chain not supported!"
#endif
int g_region_num = sizeof(g_mm_region)/sizeof(k_mm_region_t);
