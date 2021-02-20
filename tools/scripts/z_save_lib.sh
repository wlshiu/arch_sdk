#!/bin/bash -
# Copyright (c) 2019, All Rights Reserved.
# @file    z_save_lib.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

help()
{
    echo -e "usage: $0 [lib-name] [lib-header-path] [lib-obj-path] [dest-path]"
    echo -e "   e.g. $0 freertos ./middleware/third_party ./out/ ./middleware/prebuild"
    exit 1;
}

if [ $# != 4 ]; then
    help
fi

module_name=$1
lib_name=lib${module_name}.a
search_inc_dir=$2
obj_path=$3
dst_path=$4

find ${obj_path} -type f -name ${lib_name} | xargs -i cp -f {} ${dst_path}
header_path=$(find ${search_inc_dir} -type d -name 'include' -print | grep ${module_name}/include)
ut_path=$(find ${search_inc_dir} -type d -name 'test' -print | grep ${module_name}/test)

dst_path=$(echo ${dst_path} | sed 's/\/$//')

mkdir -p ${dst_path}/${module_name}/include

cp -fr ${header_path} ${dst_path}/${module_name}/
cp -fr ${ut_path} ${dst_path}/${module_name}/
