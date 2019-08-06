# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.


NAME := gap_face_detection

APP_PATH = $(PWD)/app/example/gap_face_detection
GAP8_PATH = $(PWD)/platform/mcu/gap8
TILER_PATH = $(GAP8_PATH)/autotiler
TILER_LIB = $(TILER_PATH)/lib/libtile.a
TILER_INC = $(TILER_PATH)/include


MODEL_SRC = $(APP_PATH)/FaceDetGenerator.c $(APP_PATH)/FaceDetModel.c
MODEL_GEN = FaceDetKernels FaceDetKernelsInit
MODEL_GEN_C = $(addsuffix .c, $(MODEL_GEN))
MODEL_GEN_CLEAN = $(MODEL_GEN_C) $(addsuffix .h, $(MODEL_GEN))
APP_CFLAGS += -g -D__PMSIS__
APP_CFLAGS += -Wno-unused-but-set-variable -Wno-unused-variable

PMSIS_BSP_ROOT = $(GAP8_PATH)/pmsis_bsp/

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := this a face detection application for gap8
$(NAME)_SOURCES := main.c faceDet.c FaceDetBasicKernels.c ImageDraw.c $(MODEL_GEN_C)

GLOBAL_DEFINES += AOS_NO_WIFI
GLOBAL_CFLAGS += $(APP_CFLAGS) -DUSE_CAMERA  #-DHIMAX

GLOBAL_INCLUDES += ./
GLOBAL_INCLUDES += $(TILER_INC)
GLOBAL_INCLUDES += $(PMSIS_BSP_ROOT)/include

$(info #### TILER_INC $(TILER_INC))
$(info #### GLOBAL_INCLUDES $(GLOBAL_INCLUDES))

AOS_PRE_BUILD_TARGETS += model

# Build the code generator
GenTile:
	gcc -o $(APP_PATH)/GenTile -I$(TILER_INC) $(MODEL_SRC) $(TILER_LIB) $(APP_CFLAGS)

# Run the code generator
model: GenTile
	$(APP_PATH)/GenTile -p -o $(APP_PATH)
