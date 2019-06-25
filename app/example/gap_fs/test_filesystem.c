/* PMSIS includes */
#include "pmsis.h"

/* Demo utlities includes. */
#include "bsp/fs.h"
#include "bsp/flash/hyperflash.h"

/* Variables used. */
#define BUF_SIZE         ( 1024 )
#define COUNT            ( 5 )

static fs_file_t *file;
static char buff[BUF_SIZE + 1];
static uint32_t count_done = 0;

static void check_file(void)
{
    pi_task_t task;
    int32_t size = 0;
    for (count_done = 0; count_done < COUNT; count_done++)
    {
        size = fs_read_async(file, buff, BUF_SIZE, pi_task_block(&task));
        pi_task_wait_on(&task);
        printf("%ld) Read %ld bytes : \n%s\n", count_done, size, buff);
        if (size < BUF_SIZE)
        {
            fs_seek(file, 0);
        }
    }
}

void test_fs(void)
{
    printf("Entering main controller\n");

    char name[] = "hello.txt";

    struct pi_device fs;
    struct pi_device flash;
    struct fs_conf conf;
    struct hyperflash_conf flash_conf;

    hyperflash_conf_init(&flash_conf);
    pi_open_from_conf(&flash, &flash_conf);
    if (flash_open(&flash))
    {
        printf("Error flash open !\n");
        pmsis_exit(-1);
    }

    fs_conf_init(&conf);
    conf.flash = &flash;
    pi_open_from_conf(&fs, &conf);
    if (fs_mount(&fs))
    {
        printf("Error FS mounting !\n");
        pmsis_exit(-2);
    }

    file = fs_open(&fs, name, 0);
    if (file == NULL)
    {
        printf("File open failed !\n");
        pmsis_exit(-3);
    }
    printf("File %s open success.\n", name);

    check_file();

    fs_close(file);
    printf("File %s closed.\n", name);

    fs_unmount(&fs);
    printf("FS unmounted.\n");

    pmsis_exit(0);
}

/* Program Entry. */
int application_start(int argc, char *argv[])
{
    printf("\n\n\t *** PMSIS Filesystem Test ***\n\n");
    test_fs();
    return 0;
}
