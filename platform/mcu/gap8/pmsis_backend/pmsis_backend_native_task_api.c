#include "pmsis_backend/pmsis_backend_native_task_api.h"
#include <k_api.h>
#include <aos/kernel.h>

void __os_native_mutex_lock(void *mutex)
{
    krhino_mutex_lock((kmutex_t *)&mutex, AOS_WAIT_FOREVER);
}

void __os_native_mutex_unlock(void *mutex)
{
    krhino_mutex_unlock((kmutex_t *)&mutex);
}
