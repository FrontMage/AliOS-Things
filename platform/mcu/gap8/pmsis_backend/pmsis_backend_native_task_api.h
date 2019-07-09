#ifndef __PMSIS_BACKEND_NATIVE_TASK_API__
#define __PMSIS_BACKEND_NATIVE_TASK_API__

#include "pmsis.h"
#include <k_api.h>
#include <aos/kernel.h>

#define PI_DEFAULT_STACK_SIZE 512

// temp types until backend is really done
typedef void* __os_native_task_t;

void __os_native_mutex_lock(void *mutex);

void __os_native_mutex_unlock(void *mutex);

static inline int __os_native_api_disable_irq(void)
{
    return __disable_irq();
}

static inline void __os_native_api_restore_irq(int irq_enable)
{
    __restore_irq(irq_enable);
}

static inline void __os_native_task_suspend(__os_native_task_t *task)
{
    krhino_task_del(task);
}

static inline void *__os_native_api_create_task(void (*entry)(void*),
        void *arg,
        char *name,
        int priority)
{
    ktask_t *dyn_task;
    // task is auitostarted
    krhino_task_dyn_create(&dyn_task, name, arg, priority, 0, PI_DEFAULT_STACK_SIZE, (task_entry_t)entry, 1);
    return (void*)dyn_task;
}

static inline int __os_native_api_mutex_init(pmsis_mutex_t *mutex)
{
    // allocate all ram for us
    krhino_mutex_dyn_create(&(mutex->mutex_object),"pmsis_mutex");
    if(mutex->mutex_object)
    {
        mutex->release = __os_native_mutex_unlock;
        mutex->take = __os_native_mutex_lock;
        return 0;
    }
    return -1;
}

static inline int __os_native_api_mutex_deinit(pmsis_mutex_t *mutex)
{
    krhino_mutex_dyn_del(&(mutex->mutex_object));
}
#endif
