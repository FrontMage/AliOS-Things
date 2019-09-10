# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

NAME := gap_cifar10

APP_PATH = $(PWD)/app/example/gap_cifar10
GAP8_PATH = $(PWD)/platform/mcu/gap8
TILER_PATH = $(GAP8_PATH)/autotiler
TILER_INC = $(TILER_PATH)/include
TILER_LIB = $(TILER_PATH)/lib/libtile.a
TILER_GENERATOR_PATH = $(TILER_PATH)/generators

CIFAR_GEN_PATH = $(TILER_GENERATOR_PATH)/CNN
CIFAR_KER_PATH = $(TILER_GENERATOR_PATH)/CNN

APP_SRCS += Cifar10.c Cifar10Kernels.c
APP_INC += $(TILER_INC) $(CIFAR_KER_PATH)

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

AOS_PRE_BUILD_TARGETS += model

# Build the code generator
GenTile:
	gcc -o $(APP_PATH)/GenCifar10 -I$(TILER_INC) -I$(CIFAR_GEN_PATH) $(APP_PATH)/Cifar10Model.c $(CIFAR_GEN_PATH)/CNN_Generators.c $(TILER_LIB)

# Run the code generator
Cifar10Kernels.c: GenTile
	$(APP_PATH)/GenCifar10 -o $(APP_PATH)

model: Cifar10Kernels.c
