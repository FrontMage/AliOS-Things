/*
 * realloc.c
 */

#include <stdlib.h>
#include <string.h>
#include <aos/kernel.h>
#include <k_api.h>

/* FIXME: This is cheesy, it should be fixed later */

void *realloc(void *ptr, size_t size)
{
    return aos_realloc(ptr,size);
}
