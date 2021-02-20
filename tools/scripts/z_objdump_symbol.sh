#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    z_objdump_symbol.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

help()
{
    echo -e "usage: $0 [object file (*.o)] [out file (*.objdump)]"
    exit 1;
}

if [ $# != 2 ]; then
    help
fi

arm-none-eabi-objdump -Sx $1 > $2.objdump
