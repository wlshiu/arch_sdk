#!/bin/bash -
# Copyright (c) 2019, All Rights Reserved.
# @file    untar_toolchain.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

out_path=$1
src_path=$2

cur_dir=`pwd`

cd ${out_path}
cmake -DCMAKE_TOOLCHAIN_FILE=${src_path}/toolchain.cmake -DCMAKE_BUILD_TYPE=Release ${src_path}
make

cd ${cur_dir}
