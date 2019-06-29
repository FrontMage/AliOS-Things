#if 0

#include "hyperram.h"
#include "pmsis_hal.h"

struct pi_cl_hyperram_req_s {

};

struct pi_cl_hyperram_alloc_req_s {

};

struct pi_cl_hyperram_free_req_s {

};

static inline void pi_cl_hyperram_read(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, pi_cl_hyperram_req_t *req);

static inline void pi_cl_hyperram_read_2d(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, pi_cl_hyperram_req_t *req);

static inline void pi_cl_hyperram_read_wait(pi_cl_hyperram_req_t *req);

static inline void pi_cl_hyperram_write(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, pi_cl_hyperram_req_t *req);

static inline void pi_cl_hyperram_write_2d(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, pi_cl_hyperram_req_t *req);

static inline void pi_cl_hyperram_write_wait(pi_cl_hyperram_req_t *req);

static inline void pi_cl_hyperram_copy(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, int ext2loc, pi_cl_hyperram_req_t *req);

static inline void pi_cl_hyperram_copy_2d(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, int ext2loc, pi_cl_hyperram_req_t *req);

void pi_cl_hyperram_alloc(struct pi_device *device, uint32_t size, pi_cl_hyperram_alloc_req_t *req);

void pi_cl_hyperram_free(struct pi_device *device, uint32_t chunk, uint32_t size, pi_cl_hyperram_free_req_t *req);

static inline uint32_t pi_cl_hyperram_alloc_wait(pi_cl_hyperram_alloc_req_t *req);

static inline void pi_cl_hyperram_free_wait(pi_cl_hyperram_free_req_t *req);
#endif
