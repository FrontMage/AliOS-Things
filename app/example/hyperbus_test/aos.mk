NAME := hyperbus_test

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := Hyerbus test
$(NAME)_SOURCES := hyperbus_test.c

$(NAME)_COMPONENTS += osal_aos

GLOBAL_DEFINES += AOS_NO_WIFI

GLOBAL_INCLUDES += ./
