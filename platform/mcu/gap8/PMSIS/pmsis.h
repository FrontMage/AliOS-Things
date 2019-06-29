#ifndef __PMSIS_H__
#define __PMSIS_H__

#include "pmsis_backend/implementation_specific_defines.h"
#include "pmsis_types.h"

/* Include main HAL and transitive headers. */
// pmsis_hal will include targets, which should not be used outside of hals
#include "pmsis_hal/pmsis_hal.h"
#include "pmsis_api/include/pmsis_intrsc.h"
/* Include cluster side of pmsis. */
// AliOS --> do not include before cluster is ported correctly
//#include "pmsis_cluster/cl_pmsis.h"

// include hals:
/** Core hals **/
#include "pmsis_hal/itc/pmsis_itc.h"
#include "pmsis_hal/gap_eu/pmsis_eu.h"
/** end core hals **/

/** soc hals **/
#include "pmsis_hal/soc_eu/pmsis_soc_eu.h"
#include "pmsis_hal/fll/pmsis_fll.h"
/** end soc hals **/

/** UDMA hals **/
#include "pmsis_hal/udma/udma_core.h"
#include "pmsis_hal/udma/udma_ctrl.h"
#include "pmsis_hal/udma/udma_uart.h"
#include "pmsis_hal/udma/udma_spim/udma_spim.h"
//#include "udma_i2c.h"
/** End UDMA hals **/

/* Performance Counter */
// AliOS --> do not include before perf is ported correctly
//#include "pmsis_perf.h"
/* End Performance Counter */

#endif
