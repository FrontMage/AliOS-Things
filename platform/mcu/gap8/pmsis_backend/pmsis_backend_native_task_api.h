#ifndef __PMSIS_BACKEND_NATIVE_TASK_API__
#define __PMSIS_BACKEND_NATIVE_TASK_API__

#include "pmsis.h"
#include <k_api.h>
#include <aos/kernel.h>
#include <stdio.h>

#ifndef DEBUG
#define DBG_PRINTF(...) ((void)0)
#else
#define DBG_PRINTF printf
#endif

#define PI_DEFAULT_STACK_SIZE 512

extern void platform_exit(int code);

// temp types until backend is really done
typedef void* __os_native_task_t;

void __os_native_mutex_lock(void *mutex);

void __os_native_mutex_unlock(void *mutex);

void __os_native_sem_take(void *sem);

void __os_native_sem_give(void *sem);

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
    int err = krhino_task_suspend((ktask_t*)task);
    if(err)
    {
        printf("Suspend returned error: %x\n",err);
    }
    // things must be done in that order, can't suspend a deleted task
    hal_compiler_barrier();
    err = krhino_task_dyn_del((ktask_t*)task);
    if(err)
    {
        printf("Del returned error: %x\n",err);
    }
}

static inline void __os_native_yield(void)
{
    krhino_task_yield();
}


static inline void *__os_native_api_create_task(void (*entry)(void*),
        void *arg,
        char *name,
        int priority)
{
    ktask_t *dyn_task;
    // task is auitostarted
    krhino_task_dyn_create(&dyn_task, name, arg, priority, 0, 256, (task_entry_t)entry, 1);
    return (void*)dyn_task;
}

static inline int __os_native_api_mutex_init(pmsis_mutex_t *mutex)
{
    // allocate all ram for us
    krhino_mutex_create(&(mutex->mutex_static),"pmsis_mutex");
    mutex->mutex_object = &(mutex->mutex_static);
    mutex->release = __os_native_mutex_unlock;
    mutex->take = __os_native_mutex_lock;
    return 0;
}

static inline int __os_native_api_mutex_deinit(pmsis_mutex_t *mutex)
{
    return krhino_mutex_del(&(mutex->mutex_static));
}

static inline int __os_native_api_sem_init(pi_sem_t *sem)
{
    // this allocates sem for us
    int ret = krhino_sem_create(&(sem->sem_static),"pmsis_sem",0);
    sem->sem_object = &(sem->sem_static);
    DBG_PRINTF("sem->sem_static ptr: %p\n",&sem->sem_static);
    DBG_PRINTF("[%s] sem_object=%p\n",__func__,sem->sem_object);
    sem->give = __os_native_sem_give;
    sem->take = __os_native_sem_take;
    return ret;
}

static inline int __os_native_api_sem_deinit(pi_sem_t *sem)
{
    return krhino_sem_del(&(sem->sem_static));
}

static inline void __os_native_exit(int err)
{
    platform_exit(err);
}

static inline int __os_native_kickoff(void *arg)
{
    void (*func)(void);
    func = arg;
    func();
    return 0;
}

#endif
