#include "pmsis.h"
#include "bsp/bsp.h"
#include "aos/hal/flash.h"

#define FLASH_SECTOR_SIZE (1<<13)
#define BUFF_SIZE (FLASH_SECTOR_SIZE)

#define HYPERFLASH 0
#define SPI_FLASH 1

PI_L2 unsigned char buff[BUFF_SIZE];
PI_L2 unsigned char buff_read[BUFF_SIZE];

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
    uint32_t flash_type;
} bridge_t;

bridge_t debug_struct = {0};

static int test_entry(void)
{
    __rt_debug_struct_ptr = &debug_struct;

    *(volatile uint32_t *)&debug_struct.buff_pointer = (uint32_t)buff;

    *(volatile uint32_t *)&debug_struct.gap_ready = 1;
    printf("flasher is ready\n");
    while((*(volatile uint32_t *)&debug_struct.flash_run) == 0)
    {
        pi_time_wait_us(1);
    }
    if((*(volatile uint32_t *)&debug_struct.flash_type) == SPI_FLASH)
    {// Flash is already open by OS normally
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
            hal_partition_t flash_partition;
            uint32_t off_set = 0;
            uint32_t off_set_s;
            hal_flash_addr2offset(&flash_partition, &off_set,
                    debug_struct.flash_addr);

            hal_flash_erase(flash_partition, off_set, debug_struct.flash_size);
            off_set_s = off_set;
            hal_flash_write(flash_partition, &off_set, buff,
                    debug_struct.flash_size);
            hal_flash_read(flash_partition, &off_set_s, buff_read,
                    debug_struct.flash_size);
            int errors = 0;
            for(int i = 0; i<debug_struct.flash_size; i++)
            {
                if(buff_read[i] != buff[i])
                {
                    printf("buff_read[%i] != buff[%i] (%x / %x)\n",i,i,buff_read[i], buff[i]);
                    errors++;
                    pmsis_exit(-1);
                }
            } 
        }
    }
    else
    {
        struct pi_device flash;
        struct pi_hyperflash_conf flash_conf;

        pi_hyperflash_conf_init(&flash_conf);
        pi_open_from_conf(&flash, &flash_conf);
        if (pi_flash_open(&flash))
        {
            pmsis_exit(-3);
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
            pi_flash_erase_sector(&flash, debug_struct.flash_addr);
            pi_flash_program(&flash, debug_struct.flash_addr, (void*)buff,
                    debug_struct.flash_size);
            // -------------------------------------------------------- //


        }
    }
    printf("flasher is done\n");
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
