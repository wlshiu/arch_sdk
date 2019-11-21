#!/bin/bash -
# Copyright (c) 2019, All Rights Reserved.
# @file    z_addr2line.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

help()
{
    echo -e "usage: $0 [addr2line program name] [target elf] [addresses]"
    echo -e "   e.g. $0 addr2line test.elf 00000319 0000019a"
    exit 1;
}

if [ $# -lt 3 ]; then
    help
fi

ADDR2LINE=$1
elf_file=$2
args=("$@")

for ((i = 2 ; i < $# ; i++));
do
    # ${ADDR2LINE} -e ${elf_file} -a -f ${args[$i]}
    ${ADDR2LINE} -e ${elf_file} -f ${args[$i]}
done
