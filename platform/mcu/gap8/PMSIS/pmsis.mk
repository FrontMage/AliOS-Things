GLOBAL_INCLUDES += ./PMSIS/
GLOBAL_INCLUDES += ./PMSIS/pmsis_api/include/
GLOBAL_INCLUDES += ./PMSIS/cores/TARGET_RISCV_32/

#drivers
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/uart.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/pin_config.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/uart_internal.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/pmsis_fc_event.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/pmu/pmu_gap9.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_hal/fll/fll_gap9.c

#rtos utils
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/malloc/pmsis_l2_malloc.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/malloc/malloc_internal.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/os/pmsis_task.c

GLOBAL_CFLAGS += -D__GAP8__ -DGAPUINO
