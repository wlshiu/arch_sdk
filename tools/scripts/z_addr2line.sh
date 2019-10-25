#!/bin/bash -

set -e

help()
{
    echo -e "usage: $0 [target elf] [addresses]"
    echo -e "   e.g. $0 test.elf 00000319 0000019a"
    exit 1;
}

elf_file=$1
args=("$@")

ADDR2LINE=arm-none-eabi-addr2line


for ((i = 1 ; i < $# ; i++));
do
    # ${ADDR2LINE} -e ${elf_file} -a -f ${args[$i]}
    ${ADDR2LINE} -e ${elf_file} -f ${args[$i]}
done
