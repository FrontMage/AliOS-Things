#! /bin/bash

if [[ -z "$GAP_SDK_HOME" ]]
then
    echo "Please source sourceme.sh in GAP-BUILDER or SDK.\n"
    exit -1
fi

app=${PWD##*/}
board=gapuino8                  # Gapuino
boot=jtag
cable=ftdi@digilent             # Gapuino
chip=gap
if [[ $# -gt 0 ]]
then
    while getopts ":b:" opt
    do
        case ${opt} in
            b)
                board=${OPTARG}
                ;;
            \?)
                ;;
        esac
    done
fi
. clean.sh
if [[ "$board" == "gapoc_a" ]]
then
    cable=ftdi
    # For gapoc_a
    aos make clean && aos make gap_mnist@gapoc_a -c config && aos make
else
    # For gapuino
    aos make clean && aos make gap_mnist@gapuino8 -c config && aos make
fi

binary=$GAP_SDK_HOME/build/$app@$board/binary/$app@$board.elf
verbose=2

plpbridge --cable=$cable --boot-mode=$boot --verbose=$verbose --binary=$binary --chip=$chip load ioloop reqloop start wait &
pid_bridge=$!

sleep 2

plpbridge-rt --verbose=$verbose --binary=$binary --chip=$chip &

wait $pid_bridge
