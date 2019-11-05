/*
 * Copyright (C) 2017 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */


#include <stdint.h>
#include <stdio.h>
#include "AutoTilerLib.h"
#include "CNN_Generators.h"

void Cifar10Model(unsigned int L1Memory)
{
    // SetKernelOpts(KER_OPT_NONE, KER_OPT_BUFFER_PROMOTE);
    SetInlineMode(ALWAYS_INLINE);
    // SetInlineMode(NEVER_INLINE);
    SetSymbolDynamics();

    SetSymbolNames("Cifar10_L1_Memory", "Cifar10_L2_Memory", "Cifar10_KernelDescr", "Cifar10_KernelArgs");

    SetUsedFilesNames(0, 2, "pmsis_tiling.h", "CNN_BasicKernels.h");
    SetGeneratedFilesNames("Cifar10KernelsInit.c", "Cifar10KernelsInit.h", "Cifar10Kernels.c", "Cifar10Kernels.h");

    SetL1MemorySize(L1Memory);
    LoadCNNLibrary();

    // 5x5 Convolution followed by 2x2 Max pooling. Pure SW.
    // 1 input plane [32x32], 8 output planes [14x14]
    CNN_SmallParOutFeatConvolutionPoolReLU("Conv5x5MaxPool2x2_SW_0",     HALF_WORD, HALF_WORD, HALF_WORD, HALF_WORD, FROM_L2, FROM_L3, FROM_L3, TO_L3,  1, 8, 32, 32, 5, 5, 1, 1, 0, 0, NO_PADDING, NO_RELU, 2, 2, 2, 2, NO_PADDING, NO_RELU, MAX_POOLING);
    // 5x5 Convolution followed by 2x2 Max pooling. Pure SW.
    // 8 input planes [14x14], 12 output planes [5x5]
    CNN_SmallParOutFeatConvolutionPoolReLU("Conv5x5MaxPool2x2_SW_1",     HALF_WORD, HALF_WORD, HALF_WORD, HALF_WORD, FROM_L3, FROM_L3, FROM_L3, TO_L2,  8, 12, 14, 14, 5, 5, 1, 1, 0, 0, NO_PADDING, NO_RELU, 2, 2, 2, 2, NO_PADDING, NO_RELU, MAX_POOLING);

    // Linear Layer
    // Input 12 x [5x5], Output 10
    CNN_ParOutFeatLinearLayerReLU         ("LinearLayerReLU_1",			   HALF_WORD,HALF_WORD,HALF_WORD,HALF_WORD, FROM_L2, FROM_L3, FROM_L3, TO_L2, 12, 10, NO_RELU);
}

int main(int argc, char **argv)

{
    // Parse AutoTiler options
    if (TilerParseOptions(argc, argv)) {
        printf("Failed to initialize or incorrect output arguments directory.\n"); return 1;
    }
    // Set Auto Tiler configuration, given shared L1 memory is 51200
    Cifar10Model(51200);
    // Generate code
    GenerateTilingCode();
    return 0;
}


