NAME := autotiler_v3

# Host architecture is RISC-V
HOST_ARCH := RISC-V

$(NAME)_MBINS_TYPE := kernel
$(NAME)_VERSION    := 3.0.0
$(NAME)_SUMMARY    := greenwaves technologies autotiler

$(NAME)_CFLAGS +=  -Wno-unused-variable -Wno-unused-variable #-Wno-sign-compare -Wno-pointer-sign

GLOBAL_INCLUDES		+= ./autotiler_v3/include/
GLOBAL_INCLUDES		+= ./autotiler_v3/generators/CNN/
GLOBAL_INCLUDES		+= ./autotiler_v3/generators/FFT2D/
GLOBAL_INCLUDES		+= ./autotiler_v3/generators/IntegralImg/
GLOBAL_INCLUDES		+= ./autotiler_v3/generators/MatAdd/

$(NAME)_SOURCES     +=./autotiler_v3/generators/CNN/CNN_Conv_DP_BasicKernels.c
$(NAME)_SOURCES     +=./autotiler_v3/generators/CNN/CNN_Pooling_BasicKernels.c
$(NAME)_SOURCES     +=./autotiler_v3/generators/CNN/CNN_Generators.c
$(NAME)_SOURCES     +=./autotiler_v3/generators/CNN/CNN_SoftMax.c
$(NAME)_SOURCES     +=./autotiler_v3/generators/CNN/CNN_Conv_BasicKernels.c
$(NAME)_SOURCES     +=./autotiler_v3/generators/CNN/CNN_BiasReLULinear_BasicKernels.c
$(NAME)_SOURCES     +=./autotiler_v3/generators/CNN/CNN_MatAlgebra.c
