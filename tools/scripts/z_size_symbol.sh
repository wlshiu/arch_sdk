#!/bin/bash
# Copyright (c) 2020, All Rights Reserved.
# @file    z_size_symbol.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

# you should execute at src-root

help()
{
    echo -e "usage: $0 [object file (*.o)] "
    exit 1;
}

if [ $# != 1 ]; then
    help
fi

arm-none-eabi-size -Bt -d $1

echo -e "\n\nsymbols list (dec):\n"
# arm-none-eabi-nm -C -nslS -f bsd -t x $1
arm-none-eabi-nm -C -nslS -f bsd -t d $1 | \
    grep -e "^00000000 [0-9]* " | \
    awk '{print $2 " " $3 " " $4 "    " $5}' | \
    sort -un
