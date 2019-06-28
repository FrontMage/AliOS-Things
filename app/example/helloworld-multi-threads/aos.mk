NAME := helloworld-multi-threads

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := Hello World Multi Thread
$(NAME)_SOURCES := helloworld.c

$(NAME)_COMPONENTS += osal_aos

GLOBAL_DEFINES += AOS_NO_WIFI

GLOBAL_INCLUDES += ./
