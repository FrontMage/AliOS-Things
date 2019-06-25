src = ['board.c']

aos_global_config.set('HOST_ARCH', 'ri5cy')
aos_global_config.set('HOST_MCU_FAMILY', 'mcu_gap8')
aos_global_config.set('SUPPORT_MBINS', 'no')
aos_global_config.set('HOST_MCU_NAME', 'gap8')

component = aos_board_component('gapuino8', 'gap8', src)
linux_only_targets="helloworld"
