#include <k_api.h>
#include "pmsis.h"

void krhino_idle_hook(void)
{
    //asm volatile ("wfi");
    hal_eu_evt_mask_wait(0xFFFFFFFF);
}

void krhino_init_hook(void)
{
}

void krhino_start_hook(void)
{
}

void krhino_task_create_hook(ktask_t *task)
{
    (void)task;
}

void krhino_task_del_hook(ktask_t *task, res_free_t *arg)
{
    (void)task;
    (void)arg;
}

void krhino_task_switch_hook(ktask_t *orgin, ktask_t *dest)
{
    (void)orgin;
    (void)dest;
}

void krhino_tick_hook(void)
{
}

void krhino_task_abort_hook(ktask_t *task)
{
    (void)task;
}

void krhino_mm_alloc_hook(void *mem, size_t size)
{
    (void)mem;
    (void)size;
}

void krhino_idle_pre_hook(void)
{
}
