#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    gen_img_kconfig.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

args=("$@")
out_kconfig="${args[0]}/Kconfig.test"

echo "#" > ${out_kconfig}
echo "# Automatically generated file; DO NOT EDIT." >> ${out_kconfig}
echo -e "#\n" >> ${out_kconfig}
echo -e "menu \"unittest\"\n\n  if UNITTEST\n" >> ${out_kconfig}


module_list=()
for ((i = 1 ; i < $# ; i++));
do
    if [ -d ${args[$i]} ]; then
        module_list+=($(find ${args[$i]} -type d -name 'test'))
    fi
done

for i in "${module_list[@]}"
do
    module=$(echo $i | xargs dirname | sed 's:^\(.*/\)\?\([^/]*\):\2:')
    echo -e "    config ENABLE_TEST_${module^^}" >> ${out_kconfig}
    echo -e "      bool \"enable test of $module\"" >> ${out_kconfig}
    echo -e "      default n\n" >> ${out_kconfig}

done

echo -e "\n  endif\n\nendmenu" >> ${out_kconfig}

