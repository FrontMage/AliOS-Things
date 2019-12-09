# Select the TAP core we are using --> gap8 version of adv dbg itf
tap_select gap8_adv_debug_itf
# Select the debug unit core we are using and boot mode

set ADV_DBG_UNIT_OPTS $BOOT_MODE_JTAG_SPI_F|$HI_SPEED_MODE
du_select adv_dbg_unit [expr $ADV_DBG_UNIT_OPTS]

init
