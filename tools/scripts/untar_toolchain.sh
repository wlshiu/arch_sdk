#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    untar_toolchain.sh
# @author  Wei-Lun Hsu
# @version 0.1

# set -e

toolchain_file_path=$1
output_dir=$2

filename=$(basename -- "${toolchain_file_path}")

extension=${filename##*.}
filename=${filename%.*}

if [ ${extension} = "zip" ]; then
    unzip -q ${toolchain_file_path} -d ${output_dir}
elif [ ${extension} = "bz2" ]; then
    tar -xjf ${toolchain_file_path}  -C ${output_dir}
else
    echo -e "${Red}unknown compress file ${extension} ${NC}"
    exit -1;
fi

