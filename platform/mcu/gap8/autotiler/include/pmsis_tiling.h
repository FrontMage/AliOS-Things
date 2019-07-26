#ifndef __PMSIS_TILING__
#define __PMSIS_TILING__

#include "pmsis.h"
#include "cl_dma.h"
#include "pmsis_api/include/drivers/hyperbus.h"
#include "pmsis_cluster/drivers/delegate/hyperbus/hyperbus_cl_internal.h"

extern struct pi_device *hyperram;

#define __cl_hyper_copy(a,b,c,d,e,f) \
    pi_cl_hyper_copy((struct pi_device *) a, (uint32_t) b, (void *) c, d, f, (pi_cl_hyper_req_t *) e)

#define __cl_hyper_copy_2d(a,b,c,d,e,f,g,h) \
    pi_cl_hyper_copy_2d((struct pi_device *) a, (uint32_t) b, (void *) c, d, e, f, h, (pi_cl_hyper_req_t *) g)

#define __cl_hyper_copy_wait        pi_cl_hyper_read_wait

#define CL_HYPER_EXT2LOC RX_CHANNEL
#define CL_HYPER_LOC2EXT TX_CHANNEL


static inline void __cl_dma_memcpy(uint32_t ext, uint32_t loc, uint16_t size, cl_dma_dir_e dir, uint8_t merge, cl_dma_copy_t *copy)
{
    copy->dir = dir;
    copy->merge = merge;
    copy->size = size;
    copy->id = 0;
    copy->loc = loc;
    copy->ext = ext;
    copy->stride = 0;
    copy->length = 0;
    cl_dma_memcpy(copy);
}


static inline void __cl_dma_memcpy_2d(uint32_t ext, uint32_t loc, uint16_t size, uint16_t stride, uint16_t length, cl_dma_dir_e dir, uint8_t merge, cl_dma_copy_t *copy)
{
    copy->dir = dir;
    copy->merge = merge;
    copy->size = size;
    copy->id = 0;
    copy->loc = loc;
    copy->ext = ext;
    copy->stride = stride;
    copy->length = length;
    cl_dma_memcpy_2d(copy);
}

#endif
