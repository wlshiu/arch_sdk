#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    gen_toolchain_kconfig.sh
# @author  Wei-Lun Hsu
# @version 0.1


Red='\e[0;31m'
Yellow='\e[1;33m'
Green='\e[0;32m'
NC='\e[0m'

set -e

args=("$@")
out_kconfig="${args[0]}/Kconfig.toolchain"

echo "#" > ${out_kconfig}
echo "# Automatically generated file; DO NOT EDIT." >> ${out_kconfig}
echo "#" >> ${out_kconfig}

echo "menu \"Tool-Chain Configuration\"" >> ${out_kconfig}

echo "choice TARGET_TOOLCHAIN_PATH" >> ${out_kconfig}
echo "  prompt \"Target Tool Chain\"" >> ${out_kconfig}
echo "  ---help---" >> ${out_kconfig}
echo "      Select the target tool-chain." >> ${out_kconfig}
echo -e "\n" >> ${out_kconfig}

for ((i = 1 ; i < $# ; i++));
do
    filename=$(basename -- "${args[$i]}")
    description=${filename}

    extension=${filename##*.}
    filename=${filename%.*}

    # if [ ${extension} = "zip" ]; then
    #     echo -e "${Yellow} un-tar ${extension} ${NC}"
    #     # unzip ${args[$i]}
    # elif [ ${extension} = "bz2" ]; then
    #     echo -e "${Yellow} un-tar ${extension} ${NC}"
    #     # tar -xjf ${args[$i]}
    # else
    #     echo -e "${Red}unknown compress file ${extension} ${NC}"
    #     exit -1;
    # fi

    # description=$(echo ${args[$i]} | sed 's:'"${srctree}"/'::g')

    filename=$(echo ${filename} | sed 's:-:_:g')
    filename=$(echo ${filename} | sed 's:\.:_:g')
    echo -e "config TOOLCHAIN_${filename^^}" >> ${out_kconfig}
    echo -e "  bool \"${description}\"\n" >> ${out_kconfig}
done

echo -e "config TOOLCHAIN_CUSTOMER" >> ${out_kconfig}
echo -e "  bool \"Custom tool-chain\"" >> ${out_kconfig}

echo -e "endchoice\n" >> ${out_kconfig}

echo -e "config TARGET_CUSTOMER_TOOLCHAIN" >> ${out_kconfig}
echo -e "  string \"Custom tool-chain path\" if TOOLCHAIN_CUSTOMER" >> ${out_kconfig}
echo -e "  default \"\"" >> ${out_kconfig}
echo -e "  ---help---" >> ${out_kconfig}
echo -e "    path of tool-chain\n" >> ${out_kconfig}


echo "config TARGET_TOOLCHAIN_PATH" >> ${out_kconfig}
echo "  string" >> ${out_kconfig}

for ((i = 1 ; i < $# ; i++));
do
    # parentdir=$(dirname ${args[$i]} | sed 's,^\(.*/\)\?\([^/]*\),\2,')
    filename=$(basename -- "${args[$i]}")
    filename=${filename%.*}

    filename=$(echo ${filename} | sed 's:-:_:g')
    filename=$(echo ${filename} | sed 's:\.:_:g')
    echo "  default \"${args[$i]}\" if TOOLCHAIN_${filename^^}" >> ${out_kconfig}
done

echo "  default TARGET_CUSTOMER_TOOLCHAIN if TOOLCHAIN_CUSTOMER" >> ${out_kconfig}

echo -e "\nendmenu\n" >> ${out_kconfig}

