#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    calc_crc.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

out_path=$1
input_path=$2
target=${out_path}/calc_crc32
out_name=tmp_all_header.h

help()
{
    echo "usage: $0 [work-directory] [files-path]"
    echo "  $0 out/crc freertos/include"
    exit 1;
}

if [ $# != 2 ]; then
    help
fi

if [ ! -d ${out_path} ]; then
    mkdir -p ${out_path}
fi

# if [ ! -f ${target} ]; then
#     gcc ${src_code_path}/crc32.c -o ${target}
# fi

if [ ! -d ${input_path} ]; then
    exit 0;
fi

find ${input_path} -type f -name '*.h' | xargs cat | tr -d " \t\n\r" >> ${out_path}/${out_name}

# dos2unix -q -n ${input_file} ${out_path}/${out_name}
# dos2unix -q ${out_path}/${out_name}

${target} ${out_path}/${out_name}

rm -f ${out_path}/${out_name}
