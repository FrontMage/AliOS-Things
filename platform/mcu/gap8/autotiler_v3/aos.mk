NAME := gap8_autotiler

# Host architecture is RISC-V
HOST_ARCH := RISC-V

$(NAME)_MBINS_TYPE := kernel
$(NAME)_VERSION    := 3.0.0
$(NAME)_SUMMARY    := greenwaves technologies autotiler

$(NAME)_CFLAGS +=  -w -Wno-unused-variable -Wno-unused-variable #-Wno-sign-compare -Wno-pointer-sign

GLOBAL_INCLUDES		+= ./include/
GLOBAL_INCLUDES		+= ./generators/CNN/
GLOBAL_INCLUDES		+= ./generators/FFT2D/
GLOBAL_INCLUDES		+= ./generators/IntegralImg/
GLOBAL_INCLUDES		+= ./generators/MatAdd/

$(NAME)_SOURCES     +=./generators/CNN/CNN_Conv_DP_BasicKernels.c
$(NAME)_SOURCES     +=./generators/CNN/CNN_Pooling_BasicKernels.c
$(NAME)_SOURCES     +=./generators/CNN/CNN_Generators.c
$(NAME)_SOURCES     +=./generators/CNN/CNN_SoftMax.c
$(NAME)_SOURCES     +=./generators/CNN/CNN_Conv_BasicKernels.c
$(NAME)_SOURCES     +=./generators/CNN/CNN_BiasReLULinear_BasicKernels.c
$(NAME)_SOURCES     +=./generators/CNN/CNN_MatAlgebra.c
