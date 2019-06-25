NAME := image_dump

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := image dump with openocd
$(NAME)_SOURCES := image_dump.c

$(NAME)_COMPONENTS += osal_aos

GLOBAL_DEFINES += AOS_NO_WIFI

GLOBAL_CFLAGS += -DGAPOC_NO_UART
GLOBAL_INCLUDES += ./
