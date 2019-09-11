# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

NAME := gap_fs_to_l3

APP_PATH = $(PWD)/app/example/gap_fs_to_l3_copy
GAP8_PATH = $(PWD)/platform/mcu/gap8
APP_SRCS += test_fs_to_l3_copy.c
APP_INC +=
APP_CFLAGS += -DUSE_PMSIS_BSP=1

USE_PMSIS=1

FILE_BIAS0   = coef/Cifar10_Bias0.dat
FILE_BIAS1   = coef/Cifar10_Bias1.dat
FILE_BIAS2   = coef/Cifar10_Bias2.dat
FILE_FILTER0 = coef/Cifar10_Filter0.dat
FILE_FILTER1 = coef/Cifar10_Filter1.dat
FILE_FILTER2 = coef/Cifar10_Filter2.dat

FLASH_FILES = $(FILE_BIAS0) $(FILE_BIAS1) $(FILE_BIAS2) $(FILE_FILTER0) $(FILE_FILTER1) $(FILE_FILTER2)

PLPBRIDGE_FLAGS += -f $(FLASH_FILES)

# For GVSOC
override runner_args += --config-opt=flash/fs/files=$(CURDIR)/$(FILE_BIAS0) \
                        --config-opt=flash/fs/files=$(CURDIR)/$(FILE_BIAS1) \
                        --config-opt=flash/fs/files=$(CURDIR)/$(FILE_BIAS2) \
                        --config-opt=flash/fs/files=$(CURDIR)/$(FILE_FILTER0) \
                        --config-opt=flash/fs/files=$(CURDIR)/$(FILE_FILTER1) \
                        --config-opt=flash/fs/files=$(CURDIR)/$(FILE_FILTER2)

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := this a filesystem test for gap8
$(NAME)_SOURCES := $(APP_SRCS)

GLOBAL_DEFINES += AOS_NO_WIFI
GLOBAL_CFLAGS += $(APP_CFLAGS)

GLOBAL_INCLUDES += ./

$(info #### TILER_INC $(TILER_INC))
$(info #### GLOBAL_INCLUDES $(GLOBAL_INCLUDES))
