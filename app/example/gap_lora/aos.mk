NAME := gap_lora

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := this a sample of lora over uart for gap8
$(NAME)_SOURCES := test_lora.c

GLOBAL_DEFINES += AOS_NO_WIFI
