#!/bin/bash

set -e

args=("$@")
out_kconfig="${args[0]}/Kconfig.app"

echo "#" > ${out_kconfig}
echo "# Automatically generated file; DO NOT EDIT." >> ${out_kconfig}
echo "#" >> ${out_kconfig}
echo "choice" >> ${out_kconfig}
echo "  prompt \"Target App\"" >> ${out_kconfig}
echo "  default HELLO_WORLD" >> ${out_kconfig}
echo "  ---help---" >> ${out_kconfig}
echo "      Select the target application." >> ${out_kconfig}

for ((i = 1 ; i < $# ; i++));
do
    echo "config ${args[$i]^^}" >> ${out_kconfig}
    echo "  bool \"${args[$i]}\"" >> ${out_kconfig}
done

echo "endchoice" >> ${out_kconfig}
echo "" >> ${out_kconfig}

echo "config TARGET_APP_PROJECT" >> ${out_kconfig}
echo "  string" >> ${out_kconfig}

for ((i = 1 ; i < $# ; i++));
do
    echo "  default ${args[$i]} if ${args[$i]^^}" >> ${out_kconfig}
done

echo "" >> ${out_kconfig}


for ((i = 1 ; i < $# ; i++));
do
    kconfig_path=$(find ${args[0]}/${args[$i]} -name "Kconfig")
    if [ ! -z ${kconfig_path} ]; then
        echo "if ${args[$i]^^}" >> ${out_kconfig}
        echo "  source ${kconfig_path}" >> ${out_kconfig}
        echo "endif" >> ${out_kconfig}
    fi
done

echo "" >> ${out_kconfig}
