# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

NAME := gap_fs

APP_PATH = $(PWD)/app/example/gap_fs
GAP8_PATH = $(PWD)/platform/mcu/gap8
APP_SRCS += test_filesystem.c
APP_INC +=
APP_CFLAGS += -DUSE_PMSIS_BSP=1

USE_PMSIS=1

FLASH_FILES = hello.txt

PLPBRIDGE_FLAGS += -f $(FLASH_FILES)

# For GVSOC
override runner_args += --config-opt=flash/fs/files=$(CURDIR)/$(FLASH_FILES)

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := this a filesystem test for gap8
$(NAME)_SOURCES := $(APP_SRCS)

GLOBAL_DEFINES += AOS_NO_WIFI
GLOBAL_CFLAGS += $(APP_CFLAGS)

GLOBAL_INCLUDES += ./

$(info #### TILER_INC $(TILER_INC))
$(info #### GLOBAL_INCLUDES $(GLOBAL_INCLUDES))
