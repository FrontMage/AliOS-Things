#ifndef __PMSIS_BACKEND_IMPLEMENTATION_SPECIFIC_DEFINES__
#define __PMSIS_BACKEND_IMPLEMENTATION_SPECIFIC_DEFINES__

#include "aos/kernel.h"
#include <k_api.h>


#define UART_DRIVER_DATA_IMPLEM_SPECIFC \
    kmutex_t uart_mutex;

#endif
