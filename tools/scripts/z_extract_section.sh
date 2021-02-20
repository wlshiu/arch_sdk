#!/bin/bash -
# Copyright (c) 2019, All Rights Reserved.
# @file    z_extract_section.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

help()
{
    echo -e "usage: $0 [section name] [elf file] [output name]"
    echo -e "   e.g. $0 .data ./out/test.elf data_sec.dump"
    exit 1;
}

if [ $# != 3 ]; then
    help
fi

OBJCOPY=arm-none-eabi-objcopy
sec_name=$1
elf_file=$2
out_name=$3

${OBJCOPY} --dump-section ${sec_name}=${out_name} ${elf_file}
