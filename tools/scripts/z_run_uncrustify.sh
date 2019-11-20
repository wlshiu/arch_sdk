#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    z_run_uncrustify.sh
# @author  Wei-Lun Hsu
# @version 0.1

# set -e

help()
{
    echo -e "usage: $0 [-r/-c] [rule cfg] [file list] [output path]"
    echo -e "   [file list]     the file list which will format"
    echo -e "   [rule cfg]      the config file of uncrustify"
    echo -e "   -r              replace original files"
    echo -e "   -c              check files match rule or not"
    echo -e "   [output path]   output directory of formatted files"
    exit 1;
}

if [ $# != 4 ]; then
    help
fi

uncrustify_cfg=$2
file_list=$3
output_path=$4

result_log=syntax.log

# set +e

sed -i '/\.crc\.c/d' ${file_list}
sed -i '/\.version\.c/d' ${file_list}

case $1 in
    "-c")
        ${UNCRUSTIFY} -c ${uncrustify_cfg} -F ${file_list} --check | grep --color=always 'FAIL'
        ;;
    "-r")
        ${UNCRUSTIFY} -c ${uncrustify_cfg} -F ${file_list} --prefix ${output_path} > ${output_path}/${result_log} 2>&1
        ;;
    *)

        echo "Not support option !!"
        exit
        ;;
esac

diff_ver=$(diff --version | grep 'diff (GNU' | awk -F ") " '{print $NF}' | sed 's/\.//g')

DIFF_OPTION="-p -u"
if [ ${diff_ver} \> "34" ]; then
    DIFF_OPTION="${DIFF_OPTION} --color"
fi

cat ${file_list} | xargs -i diff ${DIFF_OPTION} {} ${output_path}/{} > ${output_path}/syntax.diff
if [ -s ${output_path}/syntax.diff ]; then
    cat ${file_list} | xargs -i diff --color -p -u {} ${output_path}/{}
fi
