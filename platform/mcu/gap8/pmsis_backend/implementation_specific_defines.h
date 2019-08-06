#ifndef __PMSIS_BACKEND_IMPLEMENTATION_SPECIFIC_DEFINES__
#define __PMSIS_BACKEND_IMPLEMENTATION_SPECIFIC_DEFINES__

#include "aos/kernel.h"
#include <k_api.h>

#define __INC_TO_STRING(x) #x

#define __L2_MALLOC_NATIVE__ 1
#define pmsis_l2_malloc aos_malloc
#define pmsis_l2_malloc_free(x,y) aos_free(x)

#define IMPLEM_MUTEX_OBJECT_TYPE \
    void *mutex_object;\
    kmutex_t mutex_static;

#define IMPLEM_SEM_OBJECT_TYPE \
    void *sem_object;\
    ksem_t sem_static;

#define UART_DRIVER_DATA_IMPLEM_SPECIFC \
    kmutex_t uart_mutex_rx;\
    kmutex_t uart_mutex_tx;

// default malloc for driver structs etc (might not be compatible with udma!)
#define pi_default_malloc(x) pmsis_fc_tcdm_malloc(x)
#define pi_default_free(x,y) pmsis_fc_tcdm_malloc_free(x,y)

#define DEFAULT_MALLOC_INC  __INC_TO_STRING(rtos/malloc/pmsis_fc_tcdm_malloc.h)

// define task priorities
#define PMSIS_TASK_MAX_PRIORITY 0
#define PMSIS_TASK_OS_PRIORITY 1
#define PMSIS_TASK_USER_PRIORITY AOS_DEFAULT_APP_PRI
#define PMSIS_TASK_EVENT_KERNEL_PRIORITY (PMSIS_TASK_USER_PRIORITY-1)

#define PI_TASK_IMPLEM\
    uint32_t data[6];\
    struct pi_task *next;\
    int destroy;

#endif
