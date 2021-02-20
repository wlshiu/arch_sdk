#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    z_verify_prebuild_lib.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

Red='\e[0;31m'
Yellow='\e[1;33m'
Green='\e[0;32m'
Cyan='\e[0;36m'
NC='\e[0m' # No Color

help()
{
    echo "usage: $0 [prebuild-path] [calc-crc-shell] [work-directory-of-calc]"
    echo "  e.g. $0 ./prebuild ./tools/calc_crc32.sh ./out/crc"
    exit 1;
}

if [ $# != 3 ]; then
    help
fi

prebuild_path=$1
calc_crc_sh=$2
calc_work_dir=$3

OBJDUMP=arm-none-eabi-objdump


component_list=($(find ${prebuild_path} -maxdepth 1 -type d ! -name '*.git' ! -name '*.repo'))

for comp in "${component_list[@]}"; do
    headers_path=${comp}/include
    lib=${prebuild_path}/lib$(basename ${comp}).a

    if [ -f ${lib} ]; then
        if [ ! -d ${headers_path} ]; then
            echo -e "${Red} Wrong directory of ${comp} ${NC}"
            echo -e "${Red} '${headers_path}' not exsit ${NC}"
            break
        fi

        crc32_value=$(${OBJDUMP} -d ${lib} | grep -A 10 '_headers_crc32_' | grep '[.]word[[:space:]+]0x[a-fA-F0-9]*$' | awk '{print $(NF)}')

        rt_crc32_value=$(bash ${calc_crc_sh} ${calc_work_dir} ${headers_path})

        if [ "${crc32_value}" != "${rt_crc32_value}" ]; then
            echo -e "${Red} The header files are modified in prebuild '${headers_path}' ${NC}"
        fi
    fi
done
