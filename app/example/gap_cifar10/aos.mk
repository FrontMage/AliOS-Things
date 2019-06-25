# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

NAME := gap_cifar10

APP_SRCS += Cifar10.c Cifar10Kernels.c
#APP_INC += $(CIFAR_KER_PATH)

APP_CFLAGS += -D__PMSIS__ -mno-memcpy -fno-tree-loop-distribute-patterns  -fdata-sections -ffunction-sections
# The generated code outputs a maybe-uninitialized error which is rather difficult to suppress
# in a clean way.
APP_CFLAGS += -Wall -Wno-maybe-uninitialized -Wno-unused-but-set-variable -Wno-sign-compare

LDFLAGS +=  -flto -Wl,--gc-sections

PLPBRIDGE_FLAGS += -jtag

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := this a Cifar10 application for gap8
$(NAME)_SOURCES := $(APP_SRCS)

GLOBAL_DEFINES += AOS_NO_WIFI
GLOBAL_CFLAGS += $(APP_CFLAGS)

GLOBAL_INCLUDES += ./

$(info #### TILER_INC $(TILER_INC))
$(info #### GLOBAL_INCLUDES $(GLOBAL_INCLUDES))



#####################################################
#      Autotiler PreBuild
#####################################################

APP_PATH_ABS = $(PWD)/app/example/gap_cifar10
GAP8_PATH_ABS = $(PWD)/platform/mcu/gap8
TILER_PATH_ABS = $(GAP8_PATH_ABS)/autotiler_v3
TILER_INC_ABS = $(TILER_PATH_ABS)/include
TILER_LIB_ABS = $(TILER_PATH_ABS)/lib/libtile.a
TILER_GENERATOR_PATH_ABS = $(TILER_PATH_ABS)/generators

CIFAR_GEN_PATH_ABS = $(TILER_GENERATOR_PATH_ABS)/CNN

AOS_PRE_BUILD_TARGETS += model

# Build the code generator
GenTile:
	gcc -o $(APP_PATH_ABS)/GenCifar10 -I$(TILER_INC_ABS) -I$(CIFAR_GEN_PATH_ABS) $(APP_PATH_ABS)/Cifar10Model.c $(CIFAR_GEN_PATH_ABS)/CNN_Generators.c $(TILER_LIB_ABS) # Run the code generator

Cifar10Kernels.c: GenTile
	$(APP_PATH_ABS)/GenCifar10 -o $(APP_PATH_ABS)

model: Cifar10Kernels.c
