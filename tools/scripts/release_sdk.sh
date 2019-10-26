#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    release_sdk.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

Red='\e[0;31m'
Yellow='\e[1;33m'
Green='\e[0;32m'
Cyan='\e[0;36m'
NC='\e[0m' # No Color

root_dir=$1
out_dir=$2
out_name=$3
cur_dir=`pwd`

datetime=$(date '+%Y%m%d-%H_%M_%S')
out_name=${out_name}_${datetime}

ignore_list=(
'Kconfig.linkscript'
'Kconfig.devices'
'Kconfig.toolchain'
'Kconfig.test'
'Kconfig.app'
'**.o'
'**.d'
'**.exe'
${out_dir}
)

remove_list=(
'README.md'
'middleware/prebuild/README.md'
'configs/README.md'
'fw_img/hplc/README.md'
'tools/scripts/kconfig/conf'
'tools/scripts/kconfig/mconf'
)

if [ -d ${out_dir}/release ]; then
    rm -fr ${out_dir}/release
fi

mkdir -p ${out_dir}/release/${out_name}

echo -e "${Yellow} Output to ${out_dir}/release ${NC}"

if uname -s | grep -i linux > /dev/null; then
    CPIO=cpio
elif uname -s | grep -i MSYS > /dev/null; then
    CPIO=bsdcpio
fi

pack_list=${out_dir}/release/__pack.lst
tmp_list_1=${out_dir}/release/__tmp_1.lst


find ${root_dir} -type f \
    ! -path '*/tools/toolchain/active*' \
    ! -path '*/tools/astyle*' \
    ! -path '*/.git*' \
    ! -path '*/.repo*' > ${tmp_list_1}

cp -f ${tmp_list_1} ${pack_list}

for pattern in "${ignore_list[@]}"; do
    patt=`echo ${pattern} | sed 's:\/:\\\/:g' | sed 's:\.:\\\.:g'`
    sed -i '/'"${patt}"'/d' ${pack_list}
done

patt=`echo ${root_dir} | sed 's:\/:\\\/:g'`
sed -i 's/'"${patt}"'/./g' ${pack_list}

cat ${pack_list} | ${CPIO} -pd ${out_dir}/release/${out_name}

find ${out_dir}/release/${out_name}/device -type f ! -path '*/CMSIS/*' ! -name '*.ld' -exec rm -f {} \;

for pattern in "${remove_list[@]}"; do
    rm -f ${out_dir}/release/${out_name}/${pattern}
done

rm -f ${tmp_list_1}
rm -f ${pack_list}

echo -e "${Yellow} Pack SDK to ${out_dir}/release ${NC}"
cd ${out_dir}/release

tar -zc -f ${out_name}.tar.gz ${out_name}

cd ${cur_dir}

