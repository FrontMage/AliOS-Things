# LICENCE: GPLv2 (see COPYING)
# Copyright Greenwaves Technologies 2019
# Author: Antoine Faravelon (antoine.faravelon@greenwaves-technologies.com)

# comm struct
#  ____________________
# |    Content  | Size |
# |------0------|------|
# | HOST RDY    | (4)  |
# |-----+4------|------|
# | GAP RDY     | (4)  |
# |-----+8------|------|
# | Buff ptr    | (4)  |
# |-----+12-----|------|
# | Buff Size   | (4)  |
# |-----+16-----|------| --- 
# | FLASH RUN   | (4)  | # for flasher only
# |-----+20-----|------|
# | FLASH_ADDR  | (4)  | 
# |-----+24-----|------|
# | FLASH_SIZE  | (4)  |
# |_____________|______|

proc jtag_load_binary_and_start {file_name type} {
	puts "hello"
	load_image $file_name 0x0 $type
	# Set PC at reset vector addr
	mww 0x1B302000 0x1C000080
	# unhalt the cpu
	mww 0x1B300000 0x0
}

# gap flasher ctrl: load a bin ImageName of size ImageSize to flash at addr 0x0+flash_offset
proc gap_flasher_ctrl {device_struct_ptr_addr ImageName ImageSize flash_offset sector_size} {
	# set pointers to right addresses
	mem2array device_struct_ptr 32 $device_struct_ptr_addr 1
	set host_rdy 	    [expr $device_struct_ptr(0) + 0 ]
	set gap_rdy 	    [expr $device_struct_ptr(0) + 4 ]
	set buff_ptr_addr 	[expr $device_struct_ptr(0) + 8 ]
	set buff_size_addr 	[expr $device_struct_ptr(0) + 12 ]
	set flash_run 	    [expr $device_struct_ptr(0) + 16 ]
	set flash_addr 	    [expr $device_struct_ptr(0) + 20 ]
	set flash_size 	    [expr $device_struct_ptr(0) + 24 ]
	set size            [expr $ImageSize]
	# rest synchronisation vars
	# GAP RDY  <--- 0
	mww [expr $gap_rdy] 0x0
	# tell the chip we are going to flash
	mww [expr $flash_run] 0x1
	# HOST RDY <--- 1 / signal to begin app
	mww [expr $host_rdy] 0x1
	mem2array buff_ptr 32 $buff_ptr_addr 1
	set curr_offset [expr 0]
	puts "going to wait on addr GAP_RDY"
	while { $size > 0 } {
	if { $size > $sector_size } {
            set curr_size [expr $sector_size]
            set size [expr $size - $sector_size]
        } else {
            set curr_size [expr $size]
            set size [expr 0]
        }
        # spin on gap rdy: wait for current flash write to finish
		mem2array wait1 32 $gap_rdy 1
		while { [expr $wait1(0) != 1] } {
			mem2array wait1 32 $gap_rdy 1
			puts [expr $wait1(0)]
		}
		puts "wait on gap_rdy done witg buff ptr $buff_ptr"
		mww [expr $host_rdy] 0x0
        if { $size == 0 } {
            mww [expr $flash_run] 0x0
        }

        mww [expr $flash_addr] $curr_offset
        mww [expr $flash_size] $curr_size
        # Shift addr to the left, and set the normal base addr as min to throw
        # away bin we already read
	puts "loading image with addr [expr $buff_ptr(0) - $curr_offset] addr_min $buff_ptr(0) and size $curr_size"
        load_image $ImageName [expr $buff_ptr(0) - $curr_offset] bin $buff_ptr(0) $curr_size
        set curr_offset [expr $curr_offset + $curr_size]
		#signal app we wrote our buff
        # ACK the gap rdy now that we wrote sector (flasher may work)
		mww [expr $gap_rdy] 0x0
        #signal we are rdy when flasher is
        mww [expr $host_rdy] 0x1
	}
        # just ensure flasher app does not continue
	mem2array wait1 32 $flash_run 1
	while { [expr $wait1(0) != 1] } {
		mem2array wait1 32 $flash_run 1
		puts [expr $wait1(0)]
	}
	puts "flasher is done, exiting"
        mww [expr $gap_rdy]   0x0
        mww [expr $host_rdy]  0x0
}
