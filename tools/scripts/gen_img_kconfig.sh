#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    gen_img_kconfig.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

args=("$@")
out_kconfig="${args[0]}/Kconfig.imgs"

echo "#" > ${out_kconfig}
echo "# Automatically generated file; DO NOT EDIT." >> ${out_kconfig}
echo -e "#\n" >> ${out_kconfig}


echo "config CORE_IMG_NUM" >> ${out_kconfig}
echo "  int" >> ${out_kconfig}
echo "  default $(($# - 1))" >> ${out_kconfig}
echo "" >> ${out_kconfig}


for ((i = 1 ; i < $# ; i++));
do
    relative_path=$(echo ${args[$i]} | sed 's:'"${args[0]}/"'::g')
    # filename=$(basename -- "${args[$i]}")

    echo -e "config INSERT_CORE_IMAGE_$i" >> ${out_kconfig}
    echo -e "  bool \"Insert core image $i - ${relative_path}\"" >> ${out_kconfig}
    echo -e "  default n\n" >> ${out_kconfig}

    echo -e "  config IMAGE_NAME_$i" >> ${out_kconfig}
    echo -e "    string \"image path\"" >> ${out_kconfig}
    echo -e "    depends on INSERT_CORE_IMAGE_$i" >> ${out_kconfig}
    echo -e "    default \"${relative_path}\"\n" >> ${out_kconfig}

    echo -e "  config SECTION_NAME_$i" >> ${out_kconfig}
    echo -e "    string \"section name\"" >> ${out_kconfig}
    echo -e "    depends on INSERT_CORE_IMAGE_$i" >> ${out_kconfig}
    echo -e "    default \".core_img$i\"\n" >> ${out_kconfig}

done

echo "" >> ${out_kconfig}
