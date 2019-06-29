#if 1
#include <stdlib.h>

#include "event_kernel.h"
#include "pmsis.h"
#include "pmsis_task.h"
#include "cl_synchronisation.h"
#include "fc_to_cl_delegate.h"
#include "core_utils.h"
#include "pmsis_eu.h"

#include "pmsis_l1_malloc.h"
#include "pmsis_l2_malloc.h"

#define PRINTF(...) ((void)0)

struct pmsis_event_kernel_wrap *default_sched;

/** Internal structs for the event kernel **/
/*typedef struct pmsis_event {
    struct pmsis_event *next;
    struct pmsis_event_scheduler *sched;
    pi_task_t *fc_task;
} pmsis_event_t;*/

/**
 * event scheduler structure:
 * give a fifo for active events and a list of free events which can be used
 * by event push at any time
 **/
typedef struct pmsis_event_scheduler {
    struct pi_task *first;
    struct pi_task *last;
} pmsis_event_scheduler_t;

struct pmsis_event_kernel
{
    spinlock_t cl_to_fc_spinlock;
    pmsis_mutex_t event_sched_mutex;
    struct pmsis_event_scheduler *scheduler;
    int running;
};

static inline int pmsis_event_lock_cl_to_fc(struct pmsis_event_kernel *evt_kernel)
{
    int irq = 0;
    irq = disable_irq();
    return irq;
}

static inline void pmsis_event_unlock_cl_to_fc(struct pmsis_event_kernel *evt_kernel,
        int irq_prev)
{
    restore_irq(irq_prev);
}

static inline struct pmsis_event_kernel *pmsis_event_wrap_get_kernel(
        struct pmsis_event_kernel_wrap *wrap)
{
    struct pmsis_event_kernel *kern = (struct pmsis_event_kernel*)wrap->priv;
    return kern;
}


void pmsis_event_lock_cl_to_fc_init(struct pmsis_event_kernel_wrap *wrap)
{
    struct pmsis_event_kernel *evt_kernel = pmsis_event_wrap_get_kernel(wrap);
    cl_sync_init_spinlock(&(evt_kernel->cl_to_fc_spinlock), pmsis_l1_malloc(sizeof(uint32_t)));
    hal_compiler_barrier();
}

static inline void pmsis_event_wrap_set_kernel(
        struct pmsis_event_kernel_wrap *wrap,
        struct pmsis_event_kernel *event_kernel)
{
    wrap->priv = (void*)event_kernel;
}

static inline struct pmsis_event_scheduler *pmsis_event_wrap_get_scheduler(
        struct pmsis_event_kernel_wrap *wrap)
{
    struct pmsis_event_kernel *kern = pmsis_event_wrap_get_kernel(wrap);
    return kern->scheduler;
}

static inline void pmsis_event_wrap_set_scheduler(
        struct pmsis_event_kernel_wrap *wrap,
        struct pmsis_event_scheduler *sched)
{
    struct pmsis_event_kernel *kern = pmsis_event_wrap_get_kernel(wrap);
    kern->scheduler = sched;
}

void pmsis_event_kernel_mutex_release(struct pmsis_event_kernel_wrap *wrap)
{
    struct pmsis_event_kernel *kern = pmsis_event_wrap_get_kernel(wrap);
    pmsis_mutex_t *sched_mutex = &(kern->event_sched_mutex);
    pmsis_mutex_release(sched_mutex);
}

/**
 * Release an active event from the FIFO list
 * and push it on the free list
 * if allocated_event_nb > max_event_nb the event will be freed instead
 */
static inline void pmsis_event_release(struct pi_task *event);

/**
 * pop an event from the event FIFO, returns NULL if none available
 * MAY SLEEP
 */
static inline void pmsis_event_pop(struct pmsis_event_kernel *event_kernel,
        struct pi_task **event);

/**
 * Get an event from the scheduler free list
 * Returns NULL if none available
 */

/**** Actual implementations ****/

static inline void pmsis_event_release(struct pi_task *event)
{
    PRINTF("rt_event_release: event_ptr is %p\n",event);
    int irq = disable_irq();
    pi_task_release(event);
    restore_irq(irq);
}

static inline void pmsis_event_pop(struct pmsis_event_kernel *event_kernel,
        struct pi_task **event)
{
    int irq = __disable_irq();
    struct pmsis_event_scheduler *sched = event_kernel->scheduler;
    if(!sched->first)
    {
        *event = NULL;
        // if sched->first was null, and semaphore is taken twice, then no event
        // available, and no push is ongoing, so we'd better sleep
        __restore_irq(irq);
        pmsis_mutex_take(&event_kernel->event_sched_mutex);
        return;
    }
    // if we're here, cluster (or anything else) can't be pushing events

    // Critical section, cluster & FC might both try to be here
    // Cluster will have to go through IRQ, so locking irq is the easiest synchro
    *event = sched->first;
    sched->first = sched->first->next;
    if(sched->first == NULL)
    {// if we're at the end, reset last also
        sched->last=NULL;
    }
    __restore_irq(irq);
}

/** Might be called from cluster FC code, or ISR **/
int pmsis_event_push(struct pmsis_event_kernel_wrap *wrap, pi_task_t *event)
{
    int irq = __disable_irq();
    struct pmsis_event_kernel *event_kernel = pmsis_event_wrap_get_kernel(wrap);
    // Critical section, we are either on fc or in irq
    struct pmsis_event_scheduler* sched = pmsis_event_wrap_get_scheduler(wrap);

    event->next = NULL;

    if(sched->last)
    {
        sched->last->next = event;
        sched->last = event;
    }
    else
    {
        sched->last = event;
        sched->first = event;
    }

    // restore irqs
    // signal event kernel that a task is available
    pmsis_mutex_release(&event_kernel->event_sched_mutex);
    restore_irq(irq);
    return 0;
}

/**
 * Prepare the event kernel structure and task
 * In particular, create inner private structure
 * And setup synchronization mutexes
 */
int pmsis_event_kernel_init(struct pmsis_event_kernel_wrap **wrap,
        void (*event_kernel_entry)(void*))
{
    *wrap = pmsis_l2_malloc(sizeof(struct pmsis_event_kernel));
    struct pmsis_event_kernel_wrap *event_kernel_wrap = *wrap;
    if(!event_kernel_wrap)
    {
        return -1;
    }

    // allocate private struct and priv pointer to it
    struct pmsis_event_kernel *priv = pmsis_l2_malloc(sizeof(struct pmsis_event_kernel));
    event_kernel_wrap->priv = priv;
    if(!priv)
    {
        return -1;
    }

    struct pmsis_event_scheduler *sched = pmsis_l2_malloc(sizeof(struct pmsis_event_scheduler));
    if(!sched)
    {
        return -1;
    }

    sched->first=NULL;
    sched->last=NULL;

    pmsis_event_wrap_set_scheduler(event_kernel_wrap, sched);
    // TODO: check name for native handle
    event_kernel_wrap->__os_native_task = pmsis_task_create(event_kernel_entry,
            event_kernel_wrap,
            "event_kernel",
            PMSIS_TASK_MAX_PRIORITY);

    if(!event_kernel_wrap->__os_native_task)
    {
        return -1;
    }
    return 0;
}

static void pmsis_event_kernel_exec_event(struct pmsis_event_kernel *kernel,
        struct pi_task *event)
{
    switch(event->id)
    {
        case PI_TASK_CALLBACK_ID :
            {
                callback_t callback_func = (callback_t)event->arg[0];
                callback_func((void*)event->arg[1]);
                event->done = 1;
            }
            break;
        default: // unimplemented or mutex only
            break;
    }
}

void pmsis_event_kernel_main(void *arg)
{
    struct pmsis_event_kernel_wrap *wrap = (struct pmsis_event_kernel_wrap*)arg;
    struct pmsis_event_kernel *event_kernel = pmsis_event_wrap_get_kernel(wrap);
    // finally, initialize the mutex we'll use
    if(pmsis_mutex_init(&event_kernel->event_sched_mutex))
    {
        printf("EVENT_KERNEL: can't init mutex\n");
        return;
    }
    pmsis_mutex_take(&event_kernel->event_sched_mutex);
    event_kernel->running = 1;

    pi_task_t *event;
    while(1)
    {
        hal_compiler_barrier();
        pmsis_event_pop(event_kernel,&event);
        if(event)
        {
            pmsis_event_kernel_exec_event(event_kernel,event);
            pmsis_event_release(event);
        }
        hal_compiler_barrier();
    }
}

void pmsis_event_kernel_destroy(struct pmsis_event_kernel_wrap **wrap)
{
    struct pmsis_event_kernel *event_kernel = pmsis_event_wrap_get_kernel(*wrap);
    pmsis_task_suspend((*wrap)->__os_native_task);
}

struct pmsis_event_kernel_wrap *pmsis_event_get_default_scheduler(void)
{
    return default_sched;
}

void pmsis_event_set_default_scheduler(struct pmsis_event_kernel_wrap *wrap)
{
    default_sched = wrap;
}

void pmsis_event_destroy_default_scheduler(struct pmsis_event_kernel_wrap *wrap)
{
    pmsis_event_kernel_destroy(&wrap);
}



#endif
