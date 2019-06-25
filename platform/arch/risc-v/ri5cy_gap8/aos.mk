NAME := arch_ri5cy_gap8

$(NAME)_MBINS_TYPE := kernel
$(NAME)_VERSION    := 1.0.0
$(NAME)_SUMMARY    := arch for ri5cy

$(NAME)_SOURCES += gcc/port_s.S
$(NAME)_SOURCES += gcc/port_c.c
GLOBAL_INCLUDES += gcc/

