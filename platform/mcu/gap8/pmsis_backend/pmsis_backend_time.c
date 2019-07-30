#include <stdio.h>
#include <aos/kernel.h>

#include "k_config.h"

#define us2tick(us) \
    ((us * RHINO_CONFIG_TICKS_PER_SECOND + 999999) / 1000000)

void pi_time_wait_us(int time_us)
{
    aos_msleep(us2tick(time_us));
}
