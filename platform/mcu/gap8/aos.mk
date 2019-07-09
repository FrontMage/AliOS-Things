NAME := mcu_gap8

HOST_OPENOCD := gap8

# Host architecture is RISC-V
HOST_ARCH := RISC-V

$(NAME)_MBINS_TYPE := kernel
$(NAME)_VERSION    := 1.0.0
$(NAME)_SUMMARY    := driver & sdk for platform/mcu gap8

$(NAME)_COMPONENTS += arch_riscy
$(NAME)_COMPONENTS += rhino newlib_stub

GLOBAL_INCLUDES += aos/                               \
		   drivers/                                   \
		   gcc/                                       \
		   hal/                                       \
           tinyprintf/


#-mthumb -mthumb-interwork
GLOBAL_CFLAGS +=

GLOBAL_CFLAGS += -w

GLOBAL_ASMFLAGS += -w

#--specs=nosys.specs
GLOBAL_LDFLAGS += -Wl,--gc-sections   \
                  -nostartfiles       \
                  --specs=nosys.specs \
                  -usystem_vectors

$(NAME)_CFLAGS += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS += -Wno-unused-value -Wno-strict-aliasing

#../freedom-e-sdk/bsp/drivers/plic/plic_driver.c   \
#../aos/global_interrupts.c \

$(NAME)_SOURCES := gcc/startup_gap8.S                          \
                   gcc/gap8_iet.S                              \
                   tinyprintf/tinyprintf.c                     \
                   aos/soc_impl.c                              \
                   aos/aos.c                                   \
                   drivers/aos_uart.c                          \
                   drivers/gap_bridge.c                        \
                   pmsis_backend/event_kernel/event_kernel.c   \
                   pmsis_backend/pmsis_backend_native_task_api.c \
                   system_gap8.c

GLOBAL_LDS_FILES += platform/mcu/gap8/gcc/gap8.ld
include ./platform/mcu/gap8/PMSIS/pmsis.mk

#GLOBAL_CFLAGS += -DUSE_M_TIME -DUSE_PLIC

#GLOBAL_CFLAGS += -march=rv32imac -mabi=ilp32 -g
#GLOBAL_ASMFLAGS += -march=rv32imac -mabi=ilp32 -g
#GLOBAL_LDFLAGS += -march=rv32imac -mabi=ilp32 -g

