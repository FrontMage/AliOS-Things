NAME := gapoc_a

JTAG := jlink_jtag

HOST_ARCH           := RI5CY
HOST_MCU_FAMILY     := mcu_gap8
BOARD_NAME          := gapoc_a

CONFIG_SYSINFO_PRODUCT_MODEL := ALI_AOS_GAPOC_A
CONFIG_SYSINFO_DEVICE_NAME := gapoc_a
GLOBAL_CFLAGS += -DSYSINFO_PRODUCT_MODEL=\"$(CONFIG_SYSINFO_PRODUCT_MODEL)\"
GLOBAL_CFLAGS += -DSYSINFO_DEVICE_NAME=\"$(CONFIG_SYSINFO_DEVICE_NAME)\"
GLOBAL_CFLAGS += -DSYSINFO_ARCH=\"$(HOST_ARCH)\"
GLOBAL_CFLAGS += -DSYSINFO_MCU=\"$(HOST_MCU_FAMILY)\"
GLOBAL_CFLAGS += -DCONFIG_NO_TCPIP
GLOBAL_CFLAGS += -DTEST_CONFIG_STACK_SIZE=1024
GLOBAL_CFLAGS += -g
GLOBAL_LDFLAGS +=

GLOBAL_INCLUDES += .

GLOBAL_INCLUDES += ../../platform/mcu/gap8/
GLOBAL_INCLUDES += ../../platform/mcu/gap8/drivers/

$(NAME)_SOURCES     := ./board.c
$(NAME)_SOURCES     += ./clock_config.c
$(NAME)_SOURCES     += ./pin_mux.c
$(NAME)_SOURCES     += ./k_config.c

# include pmsis stuff
include ./platform/mcu/gap8/pmsis.mk
# configure for soc and board
GLOBAL_CFLAGS   += -D__GAP8__ -D__GAPOC_A__ -DPRINTF_USE_UART
GLOBAL_ASMFLAGS += -D__GAP8__ -D__GAPOC_A__

#TEST_COMPONENTS += certificate
EXTRA_TARGET_MAKEFILES +=  $($(BOARD_NAME)_LOCATION)/gen_image_bin.mk
