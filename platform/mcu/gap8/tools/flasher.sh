#!/bin/bash

# Copyright Greenwaves Technologies 2019
# Licence: Apache
# Script to wrap openocd flash

AOS_ROOT=$1
TARGET=$2
BOARD=$(cut -d@ -f 2 <<< $TARGET)

FLASH_IMG="$AOS_ROOT/out/$TARGET/binary/$TARGET.raw"
GAP8_PATH="$AOS_ROOT/platform/mcu/gap8"
GAP_TOOLS="$GAP8_PATH/tools"
IMG_SIZE=$(stat -c %s "$FLASH_IMG")

BOARD_SCRIPT="board/$BOARD/$BOARD.cfg"

openocd -c "script $BOARD_SCRIPT; gap_flash $FLASH_IMG $IMG_SIZE $GAP_TOOLS"
