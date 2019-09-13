# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.


NAME := gap_face_reid

APP_PATH = app/example/gap_face_reid
GAP8_PATH = platform/mcu/gap8
PMSIS_BSP_ROOT = $(GAP8_PATH)/pmsis_bsp
TILER_PATH = $(GAP8_PATH)/autotiler
TILER_LIB = $(TILER_PATH)/lib/libtile.a
TILER_INC = $(TILER_PATH)/include
TILER_GENERATOR_PATH = $(TILER_PATH)/generators
CNN_GEN_PATH = $(TILER_GENERATOR_PATH)/CNN
CNN_KER_PATH = $(TILER_GENERATOR_PATH)/CNN

APP_CFLAGS += -g -D__PMSIS__ -Os -DGAPOC=1 #-DPRINTF_GVSOC
APP_CFLAGS += -Wno-unused-but-set-variable -Wno-unused-variable

USE_PMSIS=1
USE_AUTOTILER=1


########################################
#
#		FACE DETECTION GENERATION
#
########################################
FACE_DET_PATH = .
FACE_DET_FULL_PATH = $(APP_PATH)/$(FACE_DET_PATH)
FACE_DET_MODEL_SRC = $(FACE_DET_FULL_PATH)/FaceDetGenerator.c $(FACE_DET_FULL_PATH)/FaceDetModel.c
FACE_DET_MODEL_GEN = $(FACE_DET_PATH)/FaceDetKernels
FACE_DET_MODEL_GEN_C = $(addsuffix .c, $(FACE_DET_MODEL_GEN))
FACE_DET_SRCS += $(FACE_DET_PATH)/cascade.c $(FACE_DET_PATH)/facedet_pipeline.c $(FACE_DET_PATH)/FaceDetBasicKernels.c  $(FACE_DET_MODEL_GEN_C)
FACE_DET_INCS += $(FACE_DET_PATH)

# Build the code generator
FaceDetGenTile:
	gcc -no-pie -o $(FACE_DET_FULL_PATH)/GenDetectionTile -I$(TILER_INC) $(FACE_DET_MODEL_SRC) $(TILER_LIB) $(APP_CFLAGS)

# Run the code generator
face_det_model: FaceDetGenTile
	$(FACE_DET_FULL_PATH)/GenDetectionTile -o $(FACE_DET_FULL_PATH)


########################################
#
#		FACE REID GENERATION
#
########################################
REID_PATH = .
REID_FULL_PATH = $(APP_PATH)/$(REID_PATH)
REID_MODEL_SRC = $(REID_FULL_PATH)/ReIDModel.c $(REID_FULL_PATH)/param_layer_manual.c
REID_MODEL_GEN = $(REID_PATH)/CnnKernels
REID_MODEL_GEN_C = $(addsuffix .c, $(REID_MODEL_GEN))
REID_SRCS += $(REID_PATH)/dnn_utils.c $(REID_PATH)/face_db.c $(REID_PATH)/network_process_manual.c $(REID_PATH)/reid_pipeline.c $(REID_PATH)/param_layer_manual.c #$(REID_PATH)/strangers_db.c
REID_SRCS += $(REID_MODEL_GEN_C)
REID_INCS += $(REID_PATH)

GEN_REID_TILE: $(REID_MODEL_SRC)
	gcc -no-pie -o $(REID_FULL_PATH)/GenReidNet -I$(TILER_INC) -I$(CNN_GEN_PATH) -I${CURDIR} $(REID_MODEL_SRC) $(CNN_GEN_PATH)/CNN_Generators.c $(TILER_LIB)

reid_model: GEN_REID_TILE
	$(REID_FULL_PATH)/GenReidNet -o $(REID_FULL_PATH)


########################################
#
#		RESIZING GENERATION
#
########################################
RESIZE_PATH = .
RESIZE_FULL_PATH = $(APP_PATH)/$(RESIZE_PATH)

RESIZE_MODEL_SRC = $(RESIZE_FULL_PATH)/ExtraModels.c
RESIZE_MODEL_GEN = $(RESIZE_PATH)/ExtraKernels
RESIZE_MODEL_GEN_C = $(addsuffix .c, $(RESIZE_MODEL_GEN))
#RESIZE_MODEL_GEN_CLEAN = $(RESIZE_MODEL_GEN_C) $(addsuffix .h, $(RESIZE_MODEL_GEN))
RESIZE_SRCS += $(RESIZE_MODEL_GEN_C) $(RESIZE_PATH)/ExtraBasicKernels.c
RESIZE_INCS += $(RESIZE_PATH)

GEN_RESIZE_TILE: $(RESIZE_MODEL_SRC)
	gcc -no-pie -o $(RESIZE_FULL_PATH)/GenResizeTile -I$(TILER_INC) -I$(CNN_GEN_PATH) -I${CURDIR} $(RESIZE_MODEL_SRC) $(TILER_LIB)

resizing_model: GEN_RESIZE_TILE
	$(RESIZE_FULL_PATH)/GenResizeTile -o $(RESIZE_FULL_PATH)


########################################
#
#		APPLICATION PART
#
########################################
$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := this a face reidentification application for gap8

$(NAME)_SOURCES := main.c ImageDraw.c
$(NAME)_SOURCES += $(FACE_DET_SRCS)
$(NAME)_SOURCES += $(REID_SRCS)
$(NAME)_SOURCES += $(RESIZE_SRCS)

GLOBAL_CFLAGS += $(APP_CFLAGS)

GLOBAL_INCLUDES += ./
GLOBAL_INCLUDES += $(TILER_INC)
GLOBAL_INCLUDES += $(PMSIS_BSP_ROOT)/include
GLOBAL_INCLUDES += $(FACE_DET_INCS)
GLOBAL_INCLUDES += $(REID_INCS)
GLOBAL_INCLUDES += $(RESIZE_INCS)

AOS_PRE_BUILD_TARGETS += face_det_model reid_model resizing_model

$(info #### GLOBAL_INCLUDES $(GLOBAL_INCLUDES))

GLOBAL_DEFINES += AOS_NO_WIFI
