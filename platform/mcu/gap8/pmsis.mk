GLOBAL_INCLUDES += ./PMSIS/
GLOBAL_INCLUDES += ./PMSIS/pmsis_api/include/
GLOBAL_INCLUDES += ./PMSIS/pmsis_api/include/chips/gap8/
GLOBAL_INCLUDES += ./PMSIS/cores/TARGET_RISCV_32/
GLOBAL_INCLUDES += ./pmsis_bsp/include/

#drivers
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/uart/uart.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/uart/uart_internal.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/hyper/hyperbus.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/hyper/hyperbus_internal.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/i2c/i2c.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/i2c/i2c_internal.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/cpi/cpi.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/cpi/cpi_internal.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/spi/spi.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/spi/spi_internal.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_cluster/drivers/delegate/hyperbus/hyperbus_cl_internal.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/pin_config.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/gpio/gpio.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/pad/pad.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/pmsis_fc_event.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_driver/pmu/pmu_gap8.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_hal/fll/fll_gap8.c

$(NAME)_SOURCES     += ./PMSIS/pmsis_extra/pi_fs.c
#rtos utils
# Cluster L1 memory malloc
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/malloc/pmsis_l1_malloc.c
# FC memory malloc
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/malloc/pmsis_fc_tcdm_malloc.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/malloc/pmsis_l2_malloc.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/malloc/malloc_internal.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/malloc/malloc_external.c

# task abstraction and small event kernel
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/os/pmsis_task.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/event_kernel/event_kernel.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_rtos/os/pmsis_driver_core.c

#Cluster
$(NAME)_SOURCES     += ./PMSIS/pmsis_cluster/drivers/cluster_synchronisation/fc_to_cl_delegate.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_cluster/drivers/cluster_synchronisation/cl_to_fc_delegate.c
$(NAME)_SOURCES     += ./PMSIS/pmsis_cluster/drivers/cluster_team/cl_team.c

#BSP
$(NAME)_SOURCES     += ./pmsis_bsp/bsp/gapoc_a.c
$(NAME)_SOURCES     += ./pmsis_bsp/camera/camera.c
$(NAME)_SOURCES     += ./pmsis_bsp/camera/mt9v034/mt9v034.c
$(NAME)_SOURCES     += ./pmsis_bsp/camera/himax/himax.c
#$(NAME)_SOURCES     += ./pmsis_bsp/flash/flash.c
#$(NAME)_SOURCES     += ./pmsis_bsp/fs/read_fs/read_fs.c
#$(NAME)_SOURCES     += ./pmsis_bsp/flash/hyperflash/hyperflash.c
#$(NAME)_SOURCES     += ./pmsis_bsp/transport/transport.c
#$(NAME)_SOURCES     += ./pmsis_bsp/display/display.c
#$(NAME)_SOURCES     += ./pmsis_bsp/display/ili9341/ili9341.c
#$(NAME)_SOURCES     += ./pmsis_bsp/transport/nina_w10/nina_w10.c
#$(NAME)_SOURCES     += ./pmsis_bsp/ram/hyperram/hyperram.c
#$(NAME)_SOURCES     += ./pmsis_bsp/ram/ram.c
#$(NAME)_SOURCES     += ./pmsis_bsp/ram/alloc_extern.c
$(NAME)_SOURCES     += ./pmsis_bsp/ble/nina_b112/nina_b112.c








