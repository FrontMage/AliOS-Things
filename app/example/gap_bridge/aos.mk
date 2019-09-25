NAME := gap_bridge

APP_PATH = app/example/gap_bridge
GAP8_PATH = platform/mcu/gap8

APP_SRCS += test_bridge.c $(GAP8_PATH)/drivers/gap_bridge.c
APP_INC += $(GAP8_PATH)/drivers
APP_CFLAGS +=

PLPBRIDGE_FLAGS += -fileIO

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := this a bridge test for gap8

$(NAME)_SOURCES += $(APP_SRCS)

GLOBAL_CFLAGS += $(APP_CFLAGS)

GLOBAL_INCLUDES += ./
GLOBAL_INCLUDES += $(APP_INC)

GLOBAL_DEFINES += AOS_NO_WIFI

#AOS_PRE_BUILD_TARGETS += face_det_model reid_model resizing_model

$(info #### GLOBAL_INCLUDES $(GLOBAL_INCLUDES))
