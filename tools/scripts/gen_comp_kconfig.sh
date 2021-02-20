#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    gen_comp_kconfig.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

args=("$@")
out_kconfig="${args[0]}/Kconfig.components"

echo "#" > ${out_kconfig}
echo "# Automatically generated file; DO NOT EDIT." >> ${out_kconfig}
echo -e "#\n\n" >> ${out_kconfig}

for ((i = 1 ; i < $# ; i++));
do
    echo -e "source \"${args[$i]}\"" >> ${out_kconfig}
done

echo "" >> ${out_kconfig}
