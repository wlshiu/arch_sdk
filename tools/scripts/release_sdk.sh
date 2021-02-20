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

args=("$@")

root_dir=$1
out_dir=$2
out_name=$3
save_lib_sh=$4
prebuild_dir=$5
config_prefix=$6
cur_dir=`pwd`

datetime=$(date '+%Y%m%d-%H_%M_%S')
out_name=${out_name}_${datetime}

ignore_list=(
'Kconfig.linkscript'
'Kconfig.devices'
'Kconfig.toolchain'
'Kconfig.test'
'Kconfig.imgs'
'Kconfig.app'
'Kconfig.prebuild'
'autoconfig'
'**.o'
'**.d'
# '**.exe'
'GPATH'
'GRTAGS'
'GTAGS'
'cscope*'
'tags'
'.gdb*'
'**.swp'
'**.log'
'*_defconfig'
${out_dir}
)

remove_list=(
'README.md'
'middleware/prebuild/README.md'
'configs/README.md'
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
    ! -path '*/configs*' \
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

find ${out_dir}/release/${out_name}/device -type f ! -path '*/CMSIS/*' ! -name '*.lds*' ! -name '*.release' -exec rm -f {} \;

for pattern in "${remove_list[@]}"; do
    rm -f ${out_dir}/release/${out_name}/${pattern}
done

rm -f ${tmp_list_1}
rm -f ${pack_list}

mkdir -p ${out_dir}/release/${out_name}/configs/

find ${root_dir}/apps -type f -name "${config_prefix}*_defconfig" -exec cp -f {} ${out_dir}/release/${out_name}/configs/ \;
find ${out_dir}/release/${out_name}/ -empty -type d -delete

mkdir -p ${out_dir}/release/${out_name}/middleware/prebuild/${prebuild_dir}
for ((i = 6 ; i < $# ; i++));
do
    lib_name=$(echo ${args[$i]} | xargs basename)
    if [ "${lib_name}" = "CMSIS" ]; then
        continue
    fi

    bash ${save_lib_sh} ${lib_name} ${args[$i]} ${out_dir} ${out_dir}/release/${out_name}/middleware/prebuild/${prebuild_dir}

    for k in $(find ${out_dir}/release/${out_name} -type f -name 'Kconfig.release' -print | grep /${lib_name}/); do
        dest_path=$(echo $k | sed 's:'"/${lib_name}/"': :' | awk '{print $2}' | sed "s/Kconfig\.release/Kconfig/")
        cp -f $k ${out_dir}/release/${out_name}/middleware/prebuild/${prebuild_dir}/${lib_name}/${dest_path}
    done
done

find ${out_dir}/release/${out_name}/device -type f -name '*.lds*' -exec cp -f {} ${out_dir}/release/${out_name}/middleware/prebuild/${prebuild_dir} \;

echo -e "${Yellow} Pack SDK to ${out_dir}/release ${NC}"
cd ${out_dir}/release

tar -zc -f ${out_name}.tar.gz ${out_name}

cd ${cur_dir}
