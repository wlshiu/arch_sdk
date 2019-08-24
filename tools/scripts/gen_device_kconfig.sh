#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    gen_device_kconfig.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

args=("$@")
out_kconfig="${args[0]}/Kconfig.devices"

echo "#" > ${out_kconfig}
echo "# Automatically generated file; DO NOT EDIT." >> ${out_kconfig}
echo -e "#\n" >> ${out_kconfig}

echo -e "choice" >> ${out_kconfig}
echo -e "  prompt \"Target device\"" >> ${out_kconfig}
echo -e "  ---help---" >> ${out_kconfig}
echo -e "    Select the target device.\n" >> ${out_kconfig}

for ((i = 1 ; i < $# ; i++));
do
    echo -e "config ENABLE_${args[$i]^^}" >> ${out_kconfig}
    echo -e "  bool \"${args[$i]}\"\n" >> ${out_kconfig}
done

echo -e "endchoice\n" >> ${out_kconfig}


echo "config TARGET_DEVICE" >> ${out_kconfig}
echo "  string" >> ${out_kconfig}

for ((i = 1 ; i < $# ; i++));
do
    echo "  default ${args[$i]} if ENABLE_${args[$i]^^}" >> ${out_kconfig}
done

echo "" >> ${out_kconfig}

for ((i = 1 ; i < $# ; i++));
do
    kconfig_path=$(find ${args[0]}/${args[$i]} -maxdepth 1 -name "Kconfig")
    if [ ! -z ${kconfig_path} ]; then
        echo "if ENABLE_${args[$i]^^}" >> ${out_kconfig}
        echo "  source ${kconfig_path}" >> ${out_kconfig}
        echo "endif" >> ${out_kconfig}
    fi
done

echo "" >> ${out_kconfig}
