#include "pmsis_backend/pmsis_backend_native_task_api.h"
#include <stdio.h>
#include <k_api.h>
#include <aos/kernel.h>
#include "k_mutex.h"

void __os_native_mutex_lock(void *mutex)
{
    krhino_mutex_lock((kmutex_t *)mutex, AOS_WAIT_FOREVER);
}

void __os_native_mutex_unlock(void *mutex)
{
    krhino_mutex_unlock((kmutex_t *)mutex);
}

void __os_native_sem_take(void *sem)
{
    hal_compiler_barrier();
    krhino_sem_take((ksem_t *)sem, AOS_WAIT_FOREVER);
}

void __os_native_sem_give(void *sem)
{
    krhino_sem_give((ksem_t *)sem);
}
