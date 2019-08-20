#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    gen_ld_kconfig.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

args=("$@")
out_kconfig="${args[0]}/Kconfig.linkscript"

echo "#" > ${out_kconfig}
echo "# Automatically generated file; DO NOT EDIT." >> ${out_kconfig}
echo "#" >> ${out_kconfig}

echo "menu \"Link-Script Configuration\"" >> ${out_kconfig}

echo "choice TARGET_LD_FILE" >> ${out_kconfig}
echo "  prompt \"Target Link Script\"" >> ${out_kconfig}
echo "  default LD_FILE_CMSIS_GCC_ARM" >> ${out_kconfig}
echo "  ---help---" >> ${out_kconfig}
echo "      Select the target link script." >> ${out_kconfig}
echo -e "\n" >> ${out_kconfig}

for ((i = 1 ; i < $# ; i++));
do
    parentdir=$(dirname ${args[$i]} | sed 's,^\(.*/\)\?\([^/]*\),\2,')
    filename=$(basename -- "${args[$i]}")
    filename=${filename%.*}
    description=$(echo ${args[$i]} | sed 's:'"${srctree}"/'::g')

    echo -e "config LD_FILE_${parentdir^^}_${filename^^}" >> ${out_kconfig}
    echo -e "  bool \"${description}\"\n" >> ${out_kconfig}
done

echo -e "config LD_FILE_CUSTOMER" >> ${out_kconfig}
echo -e "  bool \"Custom link script\"" >> ${out_kconfig}

echo -e "endchoice\n" >> ${out_kconfig}

echo -e "config TARGET_CUSTOMER_LD_FILE" >> ${out_kconfig}
echo -e "  string \"Custom link script file\" if LD_FILE_CUSTOMER" >> ${out_kconfig}
echo -e "  default \"\"" >> ${out_kconfig}
echo -e "  ---help---" >> ${out_kconfig}
echo -e "    e.g. xx/xxx.ld\n" >> ${out_kconfig}

echo "config TARGET_LD_FILE" >> ${out_kconfig}
echo "  string" >> ${out_kconfig}

for ((i = 1 ; i < $# ; i++));
do
    parentdir=$(dirname ${args[$i]} | sed 's,^\(.*/\)\?\([^/]*\),\2,')
    filename=$(basename -- "${args[$i]}")
    filename=${filename%.*}
    echo "  default \"${args[$i]}\" if LD_FILE_${parentdir^^}_${filename^^}" >> ${out_kconfig}
done

echo "  default TARGET_CUSTOMER_LD_FILE if LD_FILE_CUSTOMER" >> ${out_kconfig}

echo -e "\nendmenu\n" >> ${out_kconfig}

