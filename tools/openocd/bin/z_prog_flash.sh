#!/bin/sh
# Copyright (c) 2022, All Rights Reserved.
# @file    z_openocd_flash.sh
# @author  Wei-Lun Hsu
# @version 0.1

OPENOCD=Tools/openocd/bin/openocd.exe

help()
{
    echo -e "usage: $0 <target elf>"
    exit -1
}

if [ $# != 1 ]; then
    help
fi

elf_file=$1

argv="program ${elf_file} verify reset exit;"

cur_dir=$(pwd)

${OPENOCD} -s Tools/openocd/share/openocd/scripts -f interface/cmsis-dap.cfg -f target/stm32f0x.cfg -c "init;reset init;stm32f0x mass_erase 0;shutdown;"
${OPENOCD} -s Tools/openocd/share/openocd/scripts -f interface/cmsis-dap.cfg -f target/stm32f0x.cfg -c init -c "reset halt" -c "${argv}" -c shutdown

