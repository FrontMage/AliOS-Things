#include "pmsis.h"
#include "bsp/bsp.h"

#define FLASH_SECTOR_SIZE (1<<18)
#define BUFF_SIZE (FLASH_SECTOR_SIZE)

PI_L2 unsigned char buff[BUFF_SIZE];

extern void *__rt_debug_struct_ptr;

typedef struct 
{
  uint32_t host_ready;
  uint32_t gap_ready;
  uint32_t buff_pointer;
  // size is ignored here
  uint32_t buff_size;
  uint32_t flash_run;
  uint32_t flash_addr;
  uint32_t flash_size;
} bridge_t;

bridge_t debug_struct = {0};

static int test_entry(void)
{
  __rt_debug_struct_ptr = &debug_struct;
  pi_task_t task;
  uint32_t current_flash_address = 0;

  *(volatile uint32_t *)&debug_struct.buff_pointer = (uint32_t)buff;

  struct pi_device flash;
  struct hyperflash_conf flash_conf;

  hyperflash_conf_init(&flash_conf);
  pi_open_from_conf(&flash, &flash_conf);
  if (flash_open(&flash))
  {
      pmsis_exit(-3);
  }

  while((*(volatile uint32_t *)&debug_struct.flash_run) == 0)
  {
      pi_time_wait_us(1);
  }
  while(debug_struct.flash_run)
  { 
    while((*(volatile uint32_t *)&debug_struct.host_ready) == 0)
    {
      pi_time_wait_us(1);
    }

    *(volatile uint32_t *)&debug_struct.gap_ready = 1;
    // wait for ACK
    while((*(volatile uint32_t *)&debug_struct.gap_ready) == 1)
    {
      pi_time_wait_us(1);
    }

    // Erase and write the sector pointed by current_flash_addr
    flash_erase_sector(&flash, debug_struct.flash_addr);
    flash_program(&flash, debug_struct.flash_addr, (void*)buff,
            debug_struct.flash_size);
    // -------------------------------------------------------- //

    
  }
  *(volatile uint32_t *)&debug_struct.flash_run = 1;
  return 0;
}

static void test_kickoff(void *arg)
{
  int ret = test_entry();
  pmsis_exit(ret);
}

PMSIS_APP_MAIN
{
  return pmsis_kickoff((void *)test_kickoff);
}
