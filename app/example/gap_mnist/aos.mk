# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

NAME := gap_mnist

APP_PATH = app/example/gap_mnist
GAP8_PATH = platform/mcu/gap8
TILER_PATH = $(GAP8_PATH)/autotiler
TILER_INC = $(TILER_PATH)/include
TILER_LIB = $(TILER_PATH)/lib/libtile.a
TILER_GENERATOR_PATH = $(TILER_PATH)/generators

MNIST_GEN_PATH = $(TILER_GENERATOR_PATH)/CNN
MNIST_KER_PATH = $(TILER_GENERATOR_PATH)/CNN

APP_SRCS += Mnist.c MnistKernels.c \
            $(MNIST_KER_PATH)/CNN_BiasReLULinear_BasicKernels.c \
            $(MNIST_KER_PATH)/CNN_Conv_BasicKernels.c \
            $(MNIST_KER_PATH)/CNN_Pooling_BasicKernels.c \
            ImgIO.c
APP_INC += $(TILER_INC) $(MNIST_KER_PATH)
APP_CFLAGS += -DUSE_BRIDGE=1

APP_CFLAGS += -mno-memcpy -fno-tree-loop-distribute-patterns -fdata-sections -ffunction-sections
LDFLAGS +=  -flto -Wl,--gc-sections
APP_CFLAGS += -Wall -Wno-maybe-uninitialized -Wno-unused-but-set-variable

PLPBRIDGE_FLAGS += -fileIO 10

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := this a Mnist application for gap8
$(NAME)_SOURCES := $(APP_SRCS)

GLOBAL_DEFINES += AOS_NO_WIFI
GLOBAL_CFLAGS += $(APP_CFLAGS)

GLOBAL_INCLUDES += ./
GLOBAL_INCLUDES += $(APP_INC)

$(info #### TILER_INC $(TILER_INC))
$(info #### GLOBAL_INCLUDES $(GLOBAL_INCLUDES))

AOS_PRE_BUILD_TARGETS += model

# Build the code generator
GenTile:
	gcc -o $(APP_PATH)/GenMnist -I$(TILER_INC) -I$(MNIST_GEN_PATH) $(APP_PATH)/MnistModel.c $(MNIST_GEN_PATH)/CNN_Generators.c $(TILER_LIB)

# Run the code generator
MnistKernels.c: GenTile
	$(APP_PATH)/GenMnist -o $(APP_PATH)

model: MnistKernels.c
