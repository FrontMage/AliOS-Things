#include <stdlib.h>
#include "rtos/event_kernel/event_kernel.h"


int pmsis_event_push(struct pmsis_event_kernel_wrap *event_kernel, pi_task_t *task)
{
    return 0;
}


struct pmsis_event_kernel_wrap *pmsis_event_get_default_scheduler(void)
{
    return NULL;
}
