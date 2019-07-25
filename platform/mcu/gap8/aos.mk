NAME := mcu_gap8

HOST_OPENOCD := gap8

# Host architecture is RISC-V
HOST_ARCH := RISC-V

$(NAME)_MBINS_TYPE := kernel
$(NAME)_VERSION    := 1.0.0
$(NAME)_SUMMARY    := driver & sdk for platform/mcu gap8

$(NAME)_COMPONENTS += arch_ri5cy_gap8
$(NAME)_COMPONENTS += rhino

GLOBAL_INCLUDES += aos/                               \
		   drivers/                                   \
		   gcc/                                       \
		   hal/                                       \
           tinyprintf/


GLOBAL_CFLAGS += -Wno-unused-function -fdata-sections -ffunction-sections

#--specs=nosys.specs
GLOBAL_LDFLAGS += -Wl,--gc-sections   \
                  -nostartfiles       \
                  --specs=nosys.specs \
                  -usystem_vectors

$(NAME)_CFLAGS += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS += -Wno-unused-value -Wno-strict-aliasing

$(NAME)_SOURCES := gcc/startup_gap8.S                          \
                   gcc/gap8_iet.S                              \
                   tinyprintf/tinyprintf.c                     \
                   aos/soc_impl.c                              \
                   aos/aos.c                                   \
                   drivers/aos_uart.c                          \
                   drivers/gap_bridge.c                        \
                   pmsis_backend/pmsis_backend_native_task_api.c \
                   system_gap8.c

GLOBAL_LDS_FILES += platform/mcu/gap8/gcc/gap8.ld
include ./platform/mcu/gap8/pmsis.mk

GLOBAL_CFLAGS   += -D__GAP8__ -D__USE_TCDM_MALLOC__ -DFEATURE_CLUSTER -fdata-sections -ffunction-sections
GLOBAL_ASMFLAGS += -D__GAP8__ -D__USE_TCDM_MALLOC__ -DFEATURE_CLUSTER -fdata-sections -ffunction-sections

GLOBAL_CFLAGS   += -Os -march=rv32imcxgap8
GLOBAL_ASMFLAGS += -Os -march=rv32imcxgap8
GLOBAL_LDFLAGS  += -Os -march=rv32imcxgap8
