# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

NAME := gap_cifar10

APP_PATH = $(PWD)/app/example/gap_cifar10
GAP8_PATH = $(PWD)/platform/mcu/gap8
TILER_PATH = $(GAP8_PATH)/autotiler
TILER_LIB = $(TILER_PATH)/lib/libtile.a
TILER_INC = $(TILER_PATH)/include

TILER_GENERATOR_PATH = $(TILER_PATH)/generators
CIFAR_GEN_PATH = $(TILER_GENERATOR_PATH)/CNN/generator
CIFAR_KER_PATH = $(TILER_GENERATOR_PATH)/CNN/kernels

APP_SRCS += Cifar10.c Cifar10Kernels.c Cifar10KernelsInit.c \
            $(CIFAR_KER_PATH)/src/CNN_BiasReLULinear_BasicKernels.c \
            $(CIFAR_KER_PATH)/src/CNN_Conv_BasicKernels.c \
            $(CIFAR_KER_PATH)/src/CNN_Pooling_BasicKernels.c
APP_INC += $(TILER_INC) $(CIFAR_KER_PATH)/include

APP_CFLAGS += -D__PMSIS__ -mno-memcpy -fno-tree-loop-distribute-patterns  -fdata-sections -ffunction-sections
# The generated code outputs a maybe-uninitialized error which is rather difficult to suppress
# in a clean way.
APP_CFLAGS += -Wall -Wno-maybe-uninitialized -Wno-unused-but-set-variable -Wno-sign-compare

LDFLAGS +=  -flto -Wl,--gc-sections

PLPBRIDGE_FLAGS += -f binFileBuilder/coef/*
PLPBRIDGE_FLAGS += -jtag

# -DUSE_BRIDGE

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := this a Cifar10 application for gap8
$(NAME)_SOURCES := $(APP_SRCS)

GLOBAL_DEFINES += AOS_NO_WIFI
GLOBAL_CFLAGS += $(APP_CFLAGS) #-DUSE_CAMERA -DUSE_DISPLAY #-DHIMAX

GLOBAL_INCLUDES += ./
GLOBAL_INCLUDES += $(APP_INC)
#GLOBAL_INCLUDES += $(PMSIS_BSP_ROOT)/include

$(info #### TILER_INC $(TILER_INC))
$(info #### GLOBAL_INCLUDES $(GLOBAL_INCLUDES))

AOS_PRE_BUILD_TARGETS += model

# Build the code generator
GenTile:
	gcc -o $(APP_PATH)/GenCifar10 -I$(TILER_INC) -I$(CIFAR_GEN_PATH)/include $(APP_PATH)/Cifar10Model.c $(CIFAR_GEN_PATH)/src/CNN_Generators.c $(TILER_LIB)

# Run the code generator
Cifar10Kernels.c: GenTile
	$(APP_PATH)/GenCifar10 -p -o $(APP_PATH)

model: Cifar10Kernels.c
