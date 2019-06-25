#ifndef __CORE_UTILS__
#define __CORE_UTILS__

/** FC_CLUSTER_ID Definitions */
#if defined(__GAP8__)
#define FC_CLUSTER_ID                 32                /**< FC CLuster ID */
#endif

static inline uint32_t core_id() {
  int hart_id;
  asm volatile ("csrr %0, 0x014" : "=r" (hart_id) : );
  return hart_id & 0x1f;
}

static inline uint32_t cluster_id() {
  int hart_id;
  asm volatile ("csrr %0, 0x014" : "=r" (hart_id) : );
  return (hart_id >> 5) & 0x3f;
}

static inline uint32_t is_fc() {
  return ( cluster_id() == FC_CLUSTER_ID);
}

#endif
