/* PMSIS includes */
#include "pmsis.h"

/* App includes. */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gap_semihost.h"

#if defined(__PULP_OS__)
#include "bridge_stubs.h"
#endif  /* __PULP_OS__ */

/* Variables used. */
int8_t buffer[8192];

void test_bridge(void)
{
    printf("Entering main controller\n");

    uint32_t errors = 0;

    printf("MODE is %d %d %d\n",O_RDWR, O_CREAT, O_TRUNC);

    int32_t file = gap8_semihost_open("/home/antoine/Documents/AliOS-Things/app/example/openocd_semihosting/openocd_semihosting.c", O_RDWR);
    int32_t file2 = gap8_semihost_open("/home/antoine/Documents/AliOS-Things/app/example/openocd_semihosting/out_bridge.c", O_WRONLY | O_CREAT | O_TRUNC);
    if ((file == -1) || (file2 == -1))
    {
        printf("Error open file !\n");
        pmsis_exit(-1);
    }

    uint32_t size_read = 0, size_write = 0;
    while (1)
    {
        int32_t res = gap8_semihost_read(file, buffer, 8192);
        if (res == 0)
        {
            break;
        }
        size_write += res;
        gap8_semihost_write(file2, buffer, res);

        //puts(buffer);
    }
    gap8_semihost_close(file);
    gap8_semihost_close(file2);

    int32_t file3 = gap8_semihost_open("/home/antoine/Documents/AliOS-Things/app/example/openocd_semihosting/out_bridge.c", O_RDWR);
    if (file3 == -1)
    {
        printf("Error open file !\n");
        pmsis_exit(-2);
    }

    while (1)
    {
        int32_t res = gap8_semihost_read(file3, buffer, 8192);
        if (res == 0)
        {
            break;
        }
        size_read += res;
    }

    gap8_semihost_close(file3);

    errors = (size_read != size_write);

    printf("\nTest %s with %d error(s) !\n", (errors) ? "failed" : "success", errors);

    pmsis_exit(errors);
}

/* Program Entry. */
PMSIS_APP_MAIN
{
    printf("\n\n\t *** PMSIS Bridge Test ***\n\n");
    return pmsis_kickoff((void *) test_bridge);
}
