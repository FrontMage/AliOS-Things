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

/* Autotiler includes. */
#include "Gap.h"
#include "Cifar10Kernels.h"
#include "CifarData.h"

/* Variables used. */
#ifdef DEBUG
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(...) ((void) 0)
#endif

#include "coef/CifarCoeff.h"

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
    return (gap_clip(gap_roundnorm_reg(Acc, Norm), 15));
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

static void RunCifar10(void *arg)
{
    Conv5x5MaxPool2x2_SW_0(ImageIn,
                           (int16_t *) Filter_Layer0,
                           (int16_t *) Bias_Layer0,
                           (int16_t *) Out_Layer[0],
                           14);

    Conv5x5MaxPool2x2_SW_1((int16_t *) Out_Layer[0],
                           (int16_t *) Filter_Layer1,
                           (int16_t *) Bias_Layer1,
                           (int16_t *) Out_Layer[1],
                           14);

    LinearLayerReLU_1((int16_t *) Out_Layer[1],
                      (int16_t *) Filter_Layer2,
                      (int16_t *) Bias_Layer2,
                      (int16_t *) Out_Layer[2],
                      16,
                      10);
}

void test_cifar10(void)
{
    printf("Entering main controller\n");
    uint8_t CheckResults = 1;

    Out_Layer_Size[0] = (14 * 14 * sizeof(int16_t) * 8);
    Out_Layer_Size[1] = (5 * 5 * sizeof(int16_t) * 12);
    Out_Layer_Size[2] = (1 * 1 * sizeof(int16_t) * 10);

    Out_Layer[0] = (int16_t *) pmsis_l2_malloc(Out_Layer_Size[0]);
    if (Out_Layer[0] == NULL)
    {
        printf("Failed to allocated memory, giving up.\n");
        pmsis_exit(-5);
    }
    else
    {
        printf("Allocating %d: OK -> %x\n", Out_Layer_Size[0], Out_Layer[0]);
    }

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

    #if 0
    Cifar10_L2_Memory = pmsis_l2_malloc(_Cifar10_L2_Memory_SIZE);
    if (Cifar10_L2_Memory == NULL)
    {
        printf("Memory Allocation Error! Quit...\n");
        pmsis_exit(-6);
    }
    #endif
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

    pmsis_l2_malloc_free(Out_Layer[0], Out_Layer_Size[0]);
    pmsis_l2_malloc_free(Out_Layer[1], Out_Layer_Size[1]);
    pmsis_l2_malloc_free(Out_Layer[2], Out_Layer_Size[2]);

    printf("Test success\n");
    pmsis_exit(0);
}

int application_start(int argc, char *argv[])
{
    printf("\n\n\t *** Cifar10 Test ***\n\n");
    test_cifar10();
}

