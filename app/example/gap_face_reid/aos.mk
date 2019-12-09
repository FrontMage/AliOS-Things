# Copyright 2019 GreenWaves Technologies, SAS
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.



NAME := gap_face_reid

APP_PATH = app/example/gap_face_reid
GAP8_PATH = platform/mcu/gap8
PMSIS_BSP_ROOT = $(GAP8_PATH)/pmsis_bsp
TILER_PATH = $(GAP8_PATH)/autotiler_v3
TILER_LIB = $(TILER_PATH)/lib/libtile.a
TILER_INC = $(TILER_PATH)/include
TILER_GENERATOR_PATH = $(TILER_PATH)/generators
CNN_GEN_PATH = $(TILER_GENERATOR_PATH)/CNN
CNN_KER_PATH = $(TILER_GENERATOR_PATH)/CNN

APP_CFLAGS += -g -D__PMSIS__ -Os -D_FOR_GAPOC_=1 #-DPRINTF_GVSOC
APP_CFLAGS += -Wno-unused-but-set-variable -Wno-unused-variable

USE_PMSIS=1
USE_AUTOTILER=1
BOARD_NAME=gapoc_a

ifeq ($(STATIC_FACE_DB),1)
  FACE_DB_SIZE=$(shell wc -l < ./known_faces/index.txt)
  ifeq ($(FACE_DB_SIZE),)
    $(error known_faces/index.txt file doesn't exist! Please add people names there)
  endif
  ifeq ($(BLE_NOTIFIER),1)
    APP_SRCS += StaticUserManagerBleNotifier.c
    APP_CFLAGS += -DBLE_NOTIFIER=1
  else
    APP_SRCS += StaticUserManager.c
  endif

  DATA_FILES += ./known_faces/index.txt ./known_faces/*.bin
  APP_CFLAGS += -DSTATIC_FACE_DB=1 -DFACE_DB_SIZE=$(FACE_DB_SIZE)
else
  ifeq ($(BOARD_NAME),gapoc_a)
    APP_SRCS += BleUserManager.c strangers_db.c
    APP_CFLAGS += -DUSE_BLE_USER_MANAGEMENT=1
  endif
endif

ifeq ($(DUMP_SUCCESSFUL_FRAME),1)
  APP_CFLAGS += -DDUMP_SUCCESSFUL_FRAME=1
endif

ifeq ($(DUMP_SUCCESSFUL_FRAME),1)
prepare_debug:
	mkdir -p dumps
	rm -rf dumps/*
else
prepare_debug:
	true
endif

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
$(NAME)_SOURCES += $(APP_SRCS)

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
