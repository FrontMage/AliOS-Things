NAME := gapuino8

JTAG := gapuino_ftdi

HOST_ARCH           := ri5cy
HOST_MCU_FAMILY     := mcu_gap8

CONFIG_SYSINFO_PRODUCT_MODEL := ALI_AOS_GAP8
CONFIG_SYSINFO_DEVICE_NAME := gap8
GLOBAL_CFLAGS += -DSYSINFO_PRODUCT_MODEL=\"$(CONFIG_SYSINFO_PRODUCT_MODEL)\"
GLOBAL_CFLAGS += -DSYSINFO_DEVICE_NAME=\"$(CONFIG_SYSINFO_DEVICE_NAME)\"
GLOBAL_CFLAGS += -DSYSINFO_ARCH=\"$(HOST_ARCH)\"
GLOBAL_CFLAGS += -DSYSINFO_MCU=\"$(HOST_MCU_FAMILY)\"
GLOBAL_CFLAGS += -DCONFIG_NO_TCPIP
GLOBAL_CFLAGS += -DTEST_CONFIG_STACK_SIZE=1024
GLOBAL_CFLAGS += 
GLOBAL_LDFLAGS +=

GLOBAL_INCLUDES += .

GLOBAL_INCLUDES += ../../platform/mcu/gap8/
#GLOBAL_INCLUDES += ../../platform/mcu/gap8/include/
GLOBAL_INCLUDES += ../../platform/mcu/gap8/drivers/

$(NAME)_SOURCES     := ./board.c
#$(NAME)_SOURCES     += ./clock_config.c
#$(NAME)_SOURCES     += ./pin_mux.c

# include pmsis stuff
BOARD_NAME=gapuino8
#include ./platform/mcu/gap8/pmsis.mk
# configure for soc and board
GLOBAL_CFLAGS += -D__GAP8__ -DGAPUINO8 -fdata-sections -ffunction-sections -Os -g -DCONFIG_GAPUINO
GLOBAL_LDFLAGS += -Wl,--gc-sections -Os -g -DCONFIG_GAPUINO
GLOBAL_ASMFLAGS += -D__GAP8__ -DGAPUINO8 -DCONFIG_GAPUINO

GLOBAL_CFLAGS += -DHIMAX -DPRINTF_USE_UART

TEST_COMPONENTS += testcase.kernel.rhino testcase.kernel.basic aos_test
GLOBAL_CFLAGS += -DTEST_CONFIG_KV_ENABLED=0
GLOBAL_CFLAGS += -DTEST_CONFIG_YLOOP_ENABLED=0
