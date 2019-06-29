#ifndef __PMSIS_PMU_H__
#define __PMSIS_PMU_H__
#include <stdint.h>

typedef struct _pmu_state {
  uint8_t   State;
  uint8_t   FllClusterDown;
  uint8_t   DCDC_Settings[4];
  uint32_t  frequency[3];
} pmu_state_t;

#ifdef __GAP9__
#include "pmu_gap9.h"
#elif defined(__GAP8__)
// TODO:port gap8 FLL and put include here
#endif

#endif
