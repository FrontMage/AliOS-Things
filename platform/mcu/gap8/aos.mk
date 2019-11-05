NAME := mcu_gap8

HOST_OPENOCD := gap8

# Host architecture is RISC-V
HOST_ARCH := RISC-V

$(NAME)_MBINS_TYPE := kernel
$(NAME)_VERSION    := 1.0.0
$(NAME)_SUMMARY    := driver & sdk for platform/mcu gap8

$(NAME)_COMPONENTS += arch_ri5cy_gap8
$(NAME)_COMPONENTS += rhino
$(NAME)_COMPONENTS += autotiler_v3

GLOBAL_INCLUDES += aos/                               \
		   drivers/                                   \
		   gcc/                                       \
		   hal/                                       \
		   include/                                   \
           tinyprintf/


GLOBAL_CFLAGS += -Wno-unused-function -fdata-sections -ffunction-sections

#--specs=nosys.specs
GLOBAL_LDFLAGS += -Wl,--gc-sections   \
                  -nostartfiles       \
                  --specs=nosys.specs \
                  -usystem_vectors

$(NAME)_CFLAGS += -Wall -Wno-unused-variable -Wno-unused-parameter
$(NAME)_CFLAGS += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS += -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS += -Wno-unused-value -Wno-strict-aliasing

$(NAME)_SOURCES := gcc/startup_gap8.S                          \
                   gcc/gap8_iet.S                              \
                   tinyprintf/tinyprintf.c                     \
                   aos/soc_impl.c                              \
                   aos/aos.c                                   \
                   drivers/aos_uart.c                          \
                   pmsis_backend/pmsis_backend_native_task_api.c \
                   pmsis_backend/pmsis_backend_time.c \
                   src/semihost.c \
                   system_gap8.c

GLOBAL_LDS_FILES += platform/mcu/gap8/gcc/gap8.ld
include ./platform/mcu/gap8/pmsis.mk
#include ./platform/mcu/gap8/autotiler_v3.mk

GLOBAL_CFLAGS   += -D__GAP8__ -D__PMSIS__ -DPMSIS_DRIVERS -D__USE_TCDM_MALLOC__ -DFEATURE_CLUSTER -fdata-sections -ffunction-sections -DCONFIG_GAPOC_A -D__riscv__
GLOBAL_ASMFLAGS += -D__GAP8__ -D__PMSIS__ -DPMSIS_DRIVERS -D__USE_TCDM_MALLOC__ -DFEATURE_CLUSTER -fdata-sections -ffunction-sections -DCONFIG_GAPOC_A -D__riscv__

GLOBAL_CFLAGS   += -Os -g -march=rv32imcxgap8 -mPE=8 -mFC=1
GLOBAL_ASMFLAGS += -Os -g -march=rv32imcxgap8 -mPE=8 -mFC=1
GLOBAL_LDFLAGS  += -Os -g -march=rv32imcxgap8 -mPE=8 -mFC=1
