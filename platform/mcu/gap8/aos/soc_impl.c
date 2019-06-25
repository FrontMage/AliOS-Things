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
#endif

#include <aos/hal/uart.h>
extern uart_dev_t uart_0;

size_t soc_get_cur_sp()
{
    size_t sp = 0;
#if defined (__GNUC__)
	__asm volatile("mv %0,sp\n" :"=r"(sp));
#endif
    return sp;
}

krhino_err_proc_t g_err_proc = NULL;//soc_err_proc;

#if defined (__GNUC__)
extern char __heapfcram_start;
extern char __heapfcram_size;

k_mm_region_t g_mm_region[] = {
{
   (uint8_t *)&__heapfcram_start, (uint32_t)&__heapfcram_size},
};
#else
#error "Tool chain not supported!"
#endif

#include "tinyprintf.h"

void tfp_putc(void *data, char c)
{
  hal_uart_send(&uart_0, &c, 1, -1);
  asm volatile (
          "nop\t\n"
          "nop\t\n"
          "nop\t\n"
          "nop\t\n"
          "nop\t\n"
          :::);
}

// bridge to the tinyprintf
__attribute__ ((export))
int printf(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    /* Only lock the printf if the cluster is up to avoid mixing FC and cluster output */
    // ideally should lock here
    tfp_format(NULL, tfp_putc, fmt, va);
    // ideally should unlock here
    va_end(va);
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

#if 0
PUTCHAR_PROTOTYPE
{
  if (ch == '\n') {
    hal_uart_send(&uart_0, (void *)"\r", 1, 30000);
  }
  hal_uart_send(&uart_0, &ch, 1, 30000);
  return ch;
}
#endif
int g_region_num = sizeof(g_mm_region)/sizeof(k_mm_region_t);
