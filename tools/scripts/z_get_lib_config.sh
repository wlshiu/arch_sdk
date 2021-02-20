#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    z_get_lib_config.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

help()
{
    echo -e "usage: $0 [exec_objcopy] [exec_ar] [library-name] [output name]"
    echo -e "   e.g $0 arm-none-eabi-objcopy arm-none-eabi-ar libfreertos.a autoconfig.cfg"
    exit 1;
}

if [ $# != 4 ]; then
    help
fi

OBJCOPY=$1
AR=$2
lib_name=$3
out_name=$4

config_obj=__autoconfig.o
${AR} -p ${lib_name} autoconfig.o > ${config_obj}
${OBJCOPY} -I elf32-littlearm -O binary -j .data ${config_obj} ${out_name}

rm -f ${config_obj}