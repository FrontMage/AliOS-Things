#ifndef __PMSIS_BACKEND_NATIVE_TASK_API__
#define __PMSIS_BACKEND_NATIVE_TASK_API__



// temp types until backend is really done
typedef void * __os_native_task_t;

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
    assert(0);
    // TODO
}


static inline void *__os_native_api_create_task(void (*entry)(void*),
        void *arg,
        char *name,
        int priority)
{
    assert(0);
    return (void*)0;
}
#endif
