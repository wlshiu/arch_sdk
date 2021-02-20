#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    create_embitz.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

cur_dir=$(pwd)
out_dir=$1
prj_name=$2
ebp_template=$3
source_list=___src.lst
incpath_list=___inc.path
# echo ${cur_dir}

patt=$(echo ${cur_dir}/ | sed 's/^...//')

# echo -e "out_dir=${out_dir}"

mkdir -p ${out_dir}/Embitz

cd ${out_dir}/Embitz

############
## get ld file path
ld_path=$(cat ${out_dir}/autoconfig | grep 'CONFIG_TARGET_LD_FILE' | sed 's:'${patt}':@:' | \
    sed 's:\/:\\\\:g' | awk -F "@" '{print "..\\\\..\\\\" $2}' | \
    sed 's:\"::g')

############
## get include path
find ${out_dir} -type f -name '*.incp' | xargs cat | \
    awk -F "-I" '{ for (i=4; i<=NF; i++) print $i }' | \
    sed 's:'${patt}':@:' | \
    sed 's:\/:\\:g' | sed 's: ::g' | \
    awk -F "@" '{ print "<Add directory=\"..\\..\\" $2 "\" />"}' > ${incpath_list}

cp -f ${incpath_list} ${incpath_list}.tmp
sort -n ${incpath_list}.tmp | uniq > ${incpath_list}

rm -f ${incpath_list}.tmp

############
## get the source list
find ${out_dir} -type f -name '*.slst' | xargs cat | \
    sed 's:\/:\\:g' | \
    sed 's:.\\:<Unit filename=\"..\\..\\:' | \
    sed 's:\.o$:.c\"> <Option compilerVar=\"CC\" /> </Unit>:' > ${source_list}

d_list=($(find ${out_dir} -type f -name '*.d'))

for d in "${d_list[@]}"
do
    # cat ${d} | grep '[.h]:$' | sed 's:'${patt}':@:' | sed 's:\/:\\:g' | awk -F "@" '{ print "#" $2 }' | \
        # sed 's/h:/h\" \/\> /g' | sed 's:\#:<Unit filename="..\\..\\:' >> ${source_list}

    cat ${d} | grep '[.h]:$' | sed 's:'${patt}':@:' | sed 's:\/:\\:g' | awk -F "@" '{ print "<Unit filename=\"..\\..\\" $2 }' | \
        sed 's/h:/h\" \/\> /g' >> ${source_list}

done

cp -f ${source_list} ${source_list}.tmp
sort -n ${source_list}.tmp | uniq > ${source_list}

rm -f ${source_list}.tmp

############
## insert info to ebp file
cp ${ebp_template} ${prj_name}.ebp

sed -i 's/@user_project/'${prj_name}'/g' ${prj_name}.ebp
sed -i 's/@ld_path/'${ld_path}'/g' ${prj_name}.ebp

# no idea how to use ${incpath_list}...
sed -i '/@include_path/r ___inc.path' ${prj_name}.ebp
sed -i 's/@include_path//g' ${prj_name}.ebp

# no idea how to use ${source_list}...
sed -i '/@source_list/r ___src.lst' ${prj_name}.ebp
sed -i 's/@source_list/<Unit filename=\"'${ld_path}'\" \/>/g' ${prj_name}.ebp

rm -f ${incpath_list}
rm -f ${source_list}

# @include_path
# @source_list
# @ld_path
