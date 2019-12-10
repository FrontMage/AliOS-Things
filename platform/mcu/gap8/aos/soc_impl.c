/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <assert.h>
#include <aos/hal/uart.h>
#include "pmsis.h"
#include "soc_impl.h"
//#include "tinyprintf.h"
#include "stdio.h"
#include "gap_debug.h"
#include "gap_semihost.h"
#define PRINTF_BUFF_SIZE 16

extern uart_dev_t uart_0;
volatile static char g_printf_buff[PRINTF_BUFF_SIZE+1];
static kmutex_t g_printf_mutex;
static uint32_t g_printf_buff_cur_size = 0;
static int printf_is_init = 0;
static void flush_printf_buffer(void);


size_t write_uart(FILE* instance, const char *bp, size_t n);
size_t read_uart(FILE* instance, char *bp, size_t n);
int uart_flush_print_buffer(FILE *stream);
size_t write_semihost(FILE* instance, const char *bp, size_t n);
size_t read_semihost(FILE* instance, char *bp, size_t n);
int semihost_flush_print_buffer(FILE *stream);


size_t write_null(FILE* instance, const char *bp, size_t n)
{
    return 0;
}

size_t read_null(FILE* instance, char *bp, size_t n)
{
    return 0;
}

int flush_null(FILE *stream)
{
    return 0;
}


/* Standard file descriptors - implement these globals yourself. */
#if defined(PRINTF_USE_UART)
#warning "use uart"
struct File_methods uart_methods = {
    .write = write_uart,
    .read = read_uart,
    .flush  = uart_flush_print_buffer
};
FILE const i_stdin = {
    .vmt = &uart_methods,
    .fd = 0
};
FILE const i_stdout = {
    .vmt = &uart_methods,
    .fd = 1
};
FILE const i_stderr = {
    .vmt = &uart_methods,
    .fd = 2
};
#else
#if (defined(USE_SEMIHOSTING) || defined (PRINTF_GVSOC))
#warning "use semihosting"
struct File_methods semihost_methods = {
    .write = write_semihost,
    .read = read_semihost,
    .flush  = semihost_flush_print_buffer
};
FILE const i_stdin = {
    .vmt = &semihost_methods,
    .fd = 0
};
FILE const i_stdout = {
    .vmt = &semihost_methods,
    .fd = 1
};
FILE const i_stderr = {
    .vmt = &semihost_methods,
    .fd = 2
};
#else
#warning "ALL printfs will be redirected to NULL"
struct File_methods null_methods = {
    .write = write_null,
    .read = read_null,
    .flush  = flush_null
};
FILE const i_stdin = {
    .vmt = &null_methods,
    .fd = 0
};
FILE const i_stdout = {
    .vmt = &null_methods,
    .fd = 1
};
FILE const i_stderr = {
    .vmt = &null_methods,
    .fd = 2
};
#endif
#endif


FILE* const stdin  = (FILE* const)&i_stdin;
FILE* const stdout = (FILE* const)&i_stdout;
FILE* const stderr = (FILE* const)&i_stderr;

size_t write_uart(FILE* instance, const char *bp, size_t n)
{
    int ret = 0;
    if(instance->fd != 2)
    {
        for(int i=0; i<n; i++)
        {
            char c = bp[i];
            g_printf_buff[g_printf_buff_cur_size] = c;
            g_printf_buff_cur_size++;
            if((c=='\n') || (c=='\r') 
                    || (g_printf_buff_cur_size==PRINTF_BUFF_SIZE))
            {
                ret = hal_uart_send(&uart_0, g_printf_buff,
                        g_printf_buff_cur_size, -1);
                g_printf_buff_cur_size = 0;
            }
        }
    }
    else
    {
        ret = hal_uart_send(&uart_0, bp, n, -1);
    }
    return (ret ? ret : n);
}


size_t write_semihost(FILE* instance, const char *bp, size_t n)
{
#if defined(PRINTF_GVSOC)
    FC_STDOUT->PUTC[0] = c;
#else
    for(int i=0; i<n; i++)
    {
        char c = bp[i];
        g_printf_buff[g_printf_buff_cur_size] = c;
        g_printf_buff_cur_size++;
        if((c=='\n') || (c=='\r') || (g_printf_buff_cur_size==PRINTF_BUFF_SIZE))
        {
            g_printf_buff[g_printf_buff_cur_size] = '\0';
            hal_compiler_barrier();
            gap8_semihost_write0(g_printf_buff);
            hal_compiler_barrier();
            g_printf_buff_cur_size = 0;
        }
    }
#endif
    // either we're here or stuck on a breakpoint....
    return n;
}


size_t read_semihost(FILE* instance, char *bp, size_t n)
{
    gap8_semihost_read(0, bp, n);
    // failure to read n bytes will crash the chip, so if we're here, success
    return n;
}

size_t read_uart(FILE* instance, char *bp, size_t n)
{
    hal_uart_recv(&uart_0, bp, n, -1);
    // failure to read n bytes will crash the chip, so if we're here, success
    return n;
}

int uart_flush_print_buffer(FILE *stream)
{
    if(g_printf_buff_cur_size)
    {
        hal_uart_send(&uart_0, g_printf_buff, g_printf_buff_cur_size, -1);
        g_printf_buff_cur_size = 0;
    }
    return 0;
}

int semihost_flush_print_buffer(FILE *stream)
{
    if(g_printf_buff_cur_size)
    {
        g_printf_buff[g_printf_buff_cur_size] = '\0';
        gap8_semihost_write0(g_printf_buff);
        g_printf_buff_cur_size = 0;
    }
    return 0;
}

int fflush(FILE *stream)
{
    if(stream->fd != 1)
    {
        return 0;
    }
    return stream->vmt->flush(stream);
}

#if 0
void tfp_putc(void *data, char c)
{
#if (defined(PRINTF_USE_UART) && !defined(GAPOC_NO_UART))
    g_printf_buff[g_printf_buff_cur_size] = c;
    g_printf_buff_cur_size++;
    if((c=='\n') || (c=='\r') || (g_printf_buff_cur_size==PRINTF_BUFF_SIZE))
    {
        //hal_uart_send(&uart_0, g_printf_buff, g_printf_buff_cur_size, -1);
        g_printf_buff[g_printf_buff_cur_size] = '\0';
        gap8_semihost_write0(g_printf_buff);

        g_printf_buff_cur_size = 0;
    }
#elif defined(PRINTF_GVSOC)
    FC_STDOUT->PUTC[0] = c;
#else
#warning "no printf implementation"
#endif
}
#endif
#if 0
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

__attribute__ ((noreturn)) void exit(int status)
{
    platform_exit(status);
    while(1);
}

void __assert_fail(const char *asserted, const char *file, unsigned int line)
{
    printf("%s:%s:%d assert failure",asserted,file,line);
    exit(-1);
}

#if defined (__GNUC__)

extern char __heapfcram_start;
extern char __heapfcram_size;

extern char __heapl2osram_start;
extern char __heapl2osram_size;

k_mm_region_t g_mm_region[] =
{
    {(uint8_t*)&__heapfcram_start, (uint32_t)&__heapfcram_size},
    {(uint8_t*)&__heapl2osram_start, (uint32_t)&__heapl2osram_size},
};

#else
#error "Tool chain not supported!"
#endif
int g_region_num = sizeof(g_mm_region)/sizeof(k_mm_region_t);
