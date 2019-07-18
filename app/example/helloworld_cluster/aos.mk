NAME := helloworld_cluster

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := Hello World Cluster
$(NAME)_SOURCES := helloworld.c

$(NAME)_COMPONENTS += osal_aos

GLOBAL_DEFINES += AOS_NO_WIFI

GLOBAL_INCLUDES += ./
