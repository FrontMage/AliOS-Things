/*
 * Copyright (C) 2017 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */

/****************************************************************************/
/* PMSIS includes */
#include "pmsis.h"

#include "rtos/pmsis_os.h"

#include "drivers/hyperbus.h"

#include "pmsis_cluster/dma/cl_dma.h"
#include "pmsis_cluster/cluster_sync/cl_synchronisation.h"
#include "pmsis_cluster/cluster_sync/fc_to_cl_delegate.h"
#include "pmsis_cluster/cluster_team/cl_team.h"

#if defined(PMSIS_DRIVERS)
#include "pmsis_cluster/drivers/delegate/hyperbus/hyperbus_cl_internal.h"
#endif  /* PMSIS_CLUSTER */

/* Autotiler includes. */
#include "Cifar10Kernels.h"
#include "Cifar10KernelsInit.h"
#include "CifarData.h"

/* Variables used. */
#ifdef DEBUG
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(...) ((void) 0)
#endif

#ifdef COEF_L2
#include "CifarCoeff.h"
#else
#if defined(PMSIS_DRIVERS)
#include "extra/pi_fs.h"
#endif  /* PMSIS_DRIVERS */
#define  BUFFER_SIZE   1024

fs_file_t *file;
struct pi_hyper_conf conf_hyper;
struct pi_device *hyperram;
uint8_t *buff;

uint32_t Filter_Layer[3] = {0};
uint32_t Bias_Layer[3]= {0};
uint32_t Filter_Layer_Size[3] = {0};
uint32_t Bias_Layer_Size[3]= {0};

#endif  /* COEF_L2 */

int16_t *Out_Layer[3];
uint32_t Out_Layer_Size[3] = {0};

int ConvAt(short *In, short int *Filter, unsigned int X, unsigned int Y, unsigned int W, unsigned int H, unsigned int Norm)

{
    int i, j;
    int Acc = 0;
    int K = 5;

    for (i=0; i<K; i++) {
        for (j=0; j<K; j++) {
            Acc += In[(X+i)*W+Y+j]*Filter[K*i+j];
        }
    }
    return (gap8_clip(gap8_roundnorm_reg(Acc, Norm), 15));
}


void DumpPlane(char *Mess, short int *Plane, unsigned int W, unsigned int H)

{
    unsigned int i, j;

    printf("----------------- %s ------------------------\n", Mess);
    for (i=0; i<H; i++) {
        printf("%2d: ", i);
        for (j=0; j<W; j++) {
            printf("%4x ", (unsigned short) Plane[i*W+j]);
        }
        printf("\n");
    }
    printf("-----------------------------------------\n");
}

void DumpPaddedCoeff(char *Name, short int *C, unsigned int NTap, unsigned int NFilter)

{
    unsigned int i, j;
    printf("L2_MEM short int %s[] = {\n", Name);
    for (i=0; i<NFilter; i++) {
        for (j=0; j<NTap; j++) {
            printf("%d, ", C[i*NTap+j]);
        }
        printf("0,\n");
    }
    printf("};\n");
}

unsigned int CheckSum(short int *In, int Size)

{
    int i;
    unsigned S=0;

    for (i=0; i<Size; i++) S += In[i];
    return S;
}

void Check(char *Mess, short int *Planes, int NPlane, int W, int H)

{
    int i;

    printf("Check sum for %s\n", Mess);

    for (i=0; i<NPlane; i++) {
        printf("\t%2d: %x\n", i, CheckSum(Planes + i*(W*H), W*H));
    }
}

void CopyFileFromFlashToL3(fs_handle_t *fs, char *file_name, uint32_t *hyper_buff, uint32_t *hyper_size)
{
    DEBUG_PRINTF("Loading layer \"%s\" from FS to L3\n", file_name);
    file = pi_fs_open(fs, file_name, 0);
    if (file == NULL)
    {
        printf("File open failed !\n");
        pmsis_exit(-1);
    }

    *hyper_buff = (uint32_t) pi_hyperram_alloc(hyperram, file->size);
    if (!*hyper_buff)
    {
        printf("L3 malloc failed\n");
        pmsis_exit(-2);
    }
    *hyper_size = file->size;
    DEBUG_PRINTF("Hyperram alloc : %x %d\n", *hyper_buff, file->size);

    uint32_t size_total = 0;
    uint32_t size = 0;
    do{
        size = pi_fs_read(file, BUFFER_SIZE, buff);
        size = ((size + 3) & ~3);
        if (size)
        {
            pi_hyper_write(hyperram, (uint32_t) (*hyper_buff+size_total), buff, size);
        }
        size_total += size;
    } while(size_total < file->size);
    DEBUG_PRINTF("Loading layer \"%s\" from FS to L3, hyper %x size = %d\n", file_name, *hyper_buff, size_total);

    pi_fs_close(file);
}

static void RunCifar10(void *arg)
{
    //printf("Start to run Cifar10\n");
    uint8_t CheckResults = 1;

    pi_cl_hyperram_alloc_req_t alloc_req;
    pi_cl_hyperram_alloc(hyperram, Out_Layer_Size[0], &alloc_req);
    pi_cl_hyperram_alloc_wait(&alloc_req);
    Out_Layer[0] = (int16_t *) alloc_req.result;
    if (!Out_Layer[0])
    {
        printf("Hyperram alloc failed !\n");
        return;
    }

    Conv5x5MaxPool2x2_SW_0(ImageIn,
                           (int16_t *) Filter_Layer[0],
                           (int16_t *) Bias_Layer[0],
                           (int16_t *)  Out_Layer[0],
                           14,
                           0);

    Conv5x5MaxPool2x2_SW_1((int16_t *) Out_Layer[0],
                           (int16_t *) Filter_Layer[1],
                           (int16_t *) Bias_Layer[1],
                           (int16_t *)  Out_Layer[1],
                           14,
                           0);

    LinearLayerReLU_1((int16_t *) Out_Layer[1],
                      (int16_t *) Filter_Layer[2],
                      (int16_t *) Bias_Layer[2],
                      (int16_t *) Out_Layer[2],
                      16,
                      10,
                      0);

    pi_cl_hyperram_free_req_t free_req;
    pi_cl_hyperram_free(hyperram, (uint32_t) Out_Layer[0], Out_Layer_Size[0], &free_req);
    pi_cl_hyperram_free_wait(&free_req);
}

void test_cifar10(void)
{
    printf("Entering main controller\n");
    uint8_t CheckResults = 1;

    buff = (uint8_t *) pmsis_l2_malloc(BUFFER_SIZE);
    if (buff == NULL)
    {
        printf("Buffer alloc failed !\n");
        pmsis_exit(-1);
    }

    pi_hyper_conf_init(&conf_hyper);
    conf_hyper.id = 0;
    conf_hyper.ram_size = 1<<23;
    conf_hyper.type = PI_HYPER_TYPE_RAM;

    hyperram = (struct pi_device *) pmsis_l2_malloc(sizeof(struct pi_device));
    pi_open_from_conf(hyperram, &conf_hyper);
    int32_t err = pi_hyper_open(hyperram);
    if (err)
    {
        printf("Hyperram open failed %d !\n", err);
        pmsis_exit(-2);
    }

    fs_config_t conf_fs;
    pi_fs_config_default(&conf_fs);

    fs_handle_t *fs = (fs_handle_t *) pmsis_l2_malloc(sizeof(fs_handle_t));
    if (fs == NULL)
    {
        printf("Fs handle malloc failed !\n");
        pmsis_exit(-3);
    }

    err = pi_fs_mount(fs, fs_HYPER, &conf_fs);
    if (err)
    {
        printf("Error FS mounting !\n");
        pmsis_l2_malloc_free(fs, sizeof(fs_handle_t));
        pmsis_exit(-4);
    }
    printf("FS mounted.\n");

    CopyFileFromFlashToL3(fs, "Cifar10_Filter0.dat", &Filter_Layer[0], &Filter_Layer_Size[0]);
    CopyFileFromFlashToL3(fs, "Cifar10_Bias0.dat",   &Bias_Layer[0], &Bias_Layer_Size[0]);
    CopyFileFromFlashToL3(fs, "Cifar10_Filter1.dat", &Filter_Layer[1], &Filter_Layer_Size[1]);
    CopyFileFromFlashToL3(fs, "Cifar10_Bias1.dat",   &Bias_Layer[1], &Bias_Layer_Size[1]);
    CopyFileFromFlashToL3(fs, "Cifar10_Filter2.dat", &Filter_Layer[2], &Filter_Layer_Size[2]);
    CopyFileFromFlashToL3(fs, "Cifar10_Bias2.dat",   &Bias_Layer[2], &Bias_Layer_Size[2]);

    Out_Layer_Size[0] = (14 * 14 * sizeof(int16_t) * 8);
    Out_Layer_Size[1] = (5 * 5 * sizeof(int16_t) * 12);
    Out_Layer_Size[2] = (1 * 1 * sizeof(int16_t) * 10);

    Out_Layer[1] = (int16_t *) pmsis_l2_malloc(Out_Layer_Size[1]);
    Out_Layer[2] = (int16_t *) pmsis_l2_malloc(Out_Layer_Size[2]);
    if ((Out_Layer[1] == NULL) && (Out_Layer[2] == NULL))
    {
        printf("Failed to allocated memory, giving up.\n");
        pmsis_exit(-5);
    }
    else
    {
        printf("Allocating %d: OK -> %x\n", Out_Layer_Size[1], Out_Layer[1]);
        printf("Allocating %d: OK -> %x\n", Out_Layer_Size[2], Out_Layer[2]);
    }

    Cifar10_L2_Memory = pmsis_l2_malloc(_Cifar10_L2_Memory_SIZE);
    if (Cifar10_L2_Memory == NULL)
    {
        printf("Memory Allocation Error! Quit...\n");
        pmsis_exit(-6);
    }
#ifdef DMP_PAD_COEFF
    DumpPaddedCoeff("Filter_Layer0_HWCE", Filter_Layer[0], 25, 1*8);
    DumpPaddedCoeff("Filter_Layer1_HWCE", Filter_Layer[1], 25, 8*12);
#endif

    /* Configure And open cluster. */
    struct pi_device cluster_dev;
    struct cluster_driver_conf cl_conf;
    cl_conf.id = 0;
    pi_open_from_conf(&cluster_dev, (void *) &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
        printf("Cluster open failed !\n");
        pmsis_exit(-7);
    }

    /* Allocate the predetermined memory in the shared L1 memory that the cluster can act on. */
    Cifar10_L1_Memory = pmsis_l1_malloc(_Cifar10_L1_Memory_SIZE);
    if (Cifar10_L1_Memory == NULL)
    {
        printf("Memory Allocation Error! Quit...\n");
        pmsis_exit(-8);
    }

    struct pi_cluster_task *task = pmsis_l2_malloc(sizeof(struct pi_cluster_task));
    memset(task, 0, sizeof(struct pi_cluster_task));
    task->entry = RunCifar10;
    task->arg = NULL;
    task->stack_size = 2048*2;

    pi_cluster_send_task_to_cl(&cluster_dev, task);

    pmsis_l1_malloc_free(Cifar10_L1_Memory, _Cifar10_L1_Memory_SIZE);

    pi_cluster_close(&cluster_dev);

    if (CheckResults)
    {
        printf("L1: ");
        Check("SW   Layer0", Out_Layer[0], 8, 14, 14);
        printf("L2: ");
        Check("SW   Layer1", Out_Layer[1], 12, 5, 5);
        printf("L3: ");
        Check("SW   Layer2", Out_Layer[2], 10, 1, 1);
    }

    for (uint32_t i = 0; i < 3; i++)
    {
        pi_hyperram_free(hyperram, Filter_Layer[i], Filter_Layer_Size[i]);
        pi_hyperram_free(hyperram, Bias_Layer[i], Bias_Layer_Size[i]);
    }
    pmsis_l2_malloc_free(buff, BUFFER_SIZE);
    pmsis_l2_malloc_free(Out_Layer[1], Out_Layer_Size[1]);
    pmsis_l2_malloc_free(Out_Layer[2], Out_Layer_Size[2]);

    printf("Test success\n");
    pmsis_exit(0);
}

int application_start(int argc, char *argv[])
{
    #if defined(__FREERTOS__)
    printf("\n\n\t *** Cifar10 Test on FreeRTOS ***\n\n");
    #elif defined(__PULP_OS__)
    printf("\n\n\t *** Cifar10 Test on Pulp Os ***\n\n");
    #else
    printf("\n\n\t *** Cifar10 Test on Ali Os ***\n\n");
    #endif
    test_cifar10();
    //return pmsis_kickoff((void *) test_cifar10);
}

