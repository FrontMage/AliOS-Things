## Environment setup

1. Download and install the pre built toolchain: 
    a. "git clone https://github.com/GreenWaves-Technologies/gap_riscv_toolchain_ubuntu_18.git"
    b. cd ~/gap_riscv_toolchain_ubuntu_18
    c. ./install.sh
2. Download and install our openocd fork:
    a. git clone https://github.com/GreenWaves-Technologies/gap8_openocd.git (check that you are on branch gap8)
    b. cd gap8_openocd
    c. ./bootstrap
    d. ./configure --prefix=/path/yo/your/prefix
    e. make -j && make install

## Upload the binary to board

To build and upload the binary to the board, use standard alios flow:
1. Install aos cube (pip install aos-cube
2. go to your alios directory
3. aos make menuconfig to choose application and board
4. aos make && aos upload to build and upload
