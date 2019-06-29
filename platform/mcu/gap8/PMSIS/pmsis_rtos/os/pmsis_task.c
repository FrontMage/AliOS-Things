#include "rtos/pmsis_os.h"

#ifdef DEBUG
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(...) ((void) 0)
#endif  /* DEBUG */

pi_task_t *pi_task_callback(pi_task_t *callback_task, void (*func)(void *), void *arg)
{
    callback_task->id = PI_TASK_CALLBACK_ID;
    callback_task->arg[0] = (uintptr_t)func;
    callback_task->arg[1] = (uintptr_t)arg;
    callback_task->done = 0;
    pmsis_mutex_init(&(callback_task->wait_on));
    DEBUG_PRINTF("mutex init\n");
    // lock the mutex so that task may be descheduled while waiting on it
    pmsis_mutex_take(&(callback_task->wait_on));
    callback_task->destroy = 0;
    DEBUG_PRINTF("mutex taken\n");
    return callback_task;
}

pi_task_t *pi_task_callback_no_mutex(pi_task_t *callback_task, void (*func)(void *), void *arg)
{
    callback_task->id = PI_TASK_CALLBACK_ID;
    callback_task->arg[0] = (uintptr_t)func;
    callback_task->arg[1] = (uintptr_t)arg;
    callback_task->done = 0;
    //pmsis_mutex_init(&(callback_task->wait_on));
    // lock the mutex so that task may be descheduled while waiting on it
    callback_task->wait_on.mutex_object = (void*)NULL;
    callback_task->destroy = 0;
    return callback_task;
}

pi_task_t *pi_task_block(pi_task_t *callback_task)
{
    callback_task->id = PI_TASK_NONE_ID;
    callback_task->done = 0;
    pmsis_mutex_init(&(callback_task->wait_on));
    // lock the mutex so that task may be descheduled while waiting on it
    pmsis_mutex_take(&(callback_task->wait_on));
    callback_task->destroy = 0;
    return callback_task;
}

pi_task_t *pi_task_block_no_mutex(pi_task_t *callback_task)
{
    callback_task->id = PI_TASK_NONE_ID;
    callback_task->done = 0;
    callback_task->wait_on.mutex_object = (void*)NULL;
    callback_task->destroy = 0;
    // lock the mutex so that task may be descheduled while waiting on it
    return callback_task;
}

void pi_task_release(pi_task_t *task)
{
    int irq = __disable_irq();
    // if the mutex is only virtual (e.g. wait on soc event)
    task->done = 1;
    hal_compiler_barrier();
    // if the sched support semaphore/mutexes
    if(task->wait_on.mutex_object)
    {
        pmsis_mutex_release(&(task->wait_on));
    }
    hal_compiler_barrier();
    if(task->destroy)
    {
        pi_task_destroy(task);
        DEBUG_PRINTF("free task %p\n",task);
        pmsis_l2_malloc_free(task, sizeof(pi_task_t));
    }
    __restore_irq(irq);
}

void pi_task_wait_on(pi_task_t *task)
{
    // if the mutex is only virtual (e.g. wait on soc event)
    while(!task->done)
    {
        // FIXME: workaround for gcc bug
        hal_compiler_barrier();
        // if the underlying scheduler support it, deschedule the task
        pmsis_mutex_take(&task->wait_on);
        hal_compiler_barrier();
    }
    // now that the wait is done, deinit the mutex as it is purely internal
    //pmsis_mutex_deinit(&task->wait_on);
}

void pi_task_wait_on_no_mutex(pi_task_t *task)
{
    // if the mutex is only virtual (e.g. wait on soc event)
    while(!task->done)
    {
        hal_compiler_barrier();
        
        asm volatile ("nop");
        // FIXME: workaround for gcc bug
        hal_compiler_barrier();
    }
}

void pi_task_destroy(pi_task_t *task)
{
    // if the mutex is only virtual (e.g. wait on soc event)
    hal_compiler_barrier();
    // if the sched support semaphore/mutexes
    if(task->wait_on.mutex_object)
    {
        pmsis_mutex_deinit(&task->wait_on);
    }
    hal_compiler_barrier();
}
