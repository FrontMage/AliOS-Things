/*
 * malloc.c
 *
 * Very simple linked-list based malloc()/free().
 */

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <aos/kernel.h>
#include <k_api.h>

void *malloc(size_t size)
{
	return aos_malloc(size);
}

void free(void *ptr)
{
    aos_free(ptr);
}
