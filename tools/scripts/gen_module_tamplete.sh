#!/bin/bash

set -e

help()
{
    echo -e "usage: $0 [module name]"
    echo -e "   [module name]   - Use ASCII characters path"
    echo -e "                     NO Whitespace/Special-characters/Uni-code"
    exit 1;
}

if [ $# != 1 ]; then
    help
fi

module_name=$1
output_kconfig=Kconfig
output_mk=component.mk

gen_kconfig()
{
    echo -e "#\n# For a description of the syntax of this configuration file," > ${output_kconfig}
    echo -e "# see doc/kconfig-language.txt.\n#\n" >> ${output_kconfig}

    echo -e "menu \"${module_name}\"\n" >> ${output_kconfig}

    echo -e "    config ENABLE_${module_name^^}" >> ${output_kconfig}
    echo -e "        bool \"enable ${module_name} module\"" >> ${output_kconfig}
    echo -e "        default n" >> ${output_kconfig}
    echo -e "        ---help---\n" >> ${output_kconfig}
    echo -e "            enable ${module_name} or not\n\n" >> ${output_kconfig}

    echo -e "    if ENABLE_${module_name^^}\n\n" >> ${output_kconfig}

    echo -e "    endif\n" >> ${output_kconfig}
    echo -e "endmenu\n" >> ${output_kconfig}
}

gen_mk()
{
    echo -e "#\n# component Makefile\n#\n\n" > ${output_mk}
    echo -e "COMPONENT_ADD_LDFLAGS +=\n" >> ${output_mk}
    echo -e "COMPONENT_ADD_LDFLAGS +=\n" >> ${output_mk}
    echo -e "COMPONENT_ADD_INCLUDEDIRS +=\n" >> ${output_mk}
    echo -e "COMPONENT_PRIV_INCLUDEDIRS +=\n" >> ${output_mk}
    echo -e "COMPONENT_OBJS :=\n" >> ${output_mk}
    echo -e "COMPONENT_SRCDIRS +=\n" >> ${output_mk}
}

if [ ! -d ${module_name} ]; then
    mkdir ${module_name}
fi

cd ${module_name}

gen_kconfig
gen_mk