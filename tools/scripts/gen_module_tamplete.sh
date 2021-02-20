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
    echo -e "        ---help---" >> ${output_kconfig}
    echo -e "            enable ${module_name} or not\n\n" >> ${output_kconfig}

    echo -e "    if ENABLE_${module_name^^}\n\n" >> ${output_kconfig}

    echo -e "    endif\n" >> ${output_kconfig}
    echo -e "endmenu\n" >> ${output_kconfig}
}

gen_mk()
{
    echo -e "#\n# component Makefile\n#\n\n" > ${output_mk}

    echo "###" >> ${output_mk}
    echo "# Add linker arguments to the LDFLAGS for the app executable. Defaults to -l\$(COMPONENT_NAME). " >> ${output_mk}
    echo "# If adding pre-compiled libraries to this directory, add them as absolute paths - ie \$(COMPONENT_PATH)/libwhatever.a" >> ${output_mk}
    echo -e "COMPONENT_ADD_LDFLAGS +=\n" >> ${output_mk}

    echo "###" >> ${output_mk}
    echo "# Paths, relative to the component directory, which will be added to the include search path for all components in the project." >> ${output_mk}
    echo "# Defaults to include if not overridden." >> ${output_mk}
    echo "# If an include directory is only needed to compile this specific component, add it to COMPONENT_PRIV_INCLUDEDIRS instead." >> ${output_mk}
    echo -e "COMPONENT_ADD_INCLUDEDIRS +=\n" >> ${output_mk}

    echo "###" >> ${output_mk}
    echo "# Directory paths, must be relative to the component directory, " >> ${output_mk}
    echo "# which will be added to the include search path for this component's source files only." >> ${output_mk}
    echo -e "COMPONENT_PRIV_INCLUDEDIRS +=\n" >> ${output_mk}

    echo "###" >> ${output_mk}
    echo "# Object files to compile. Default value is a .o file for each source file that is found in COMPONENT_SRCDIRS." >> ${output_mk}
    echo "# Overriding this list allows you to exclude source files in COMPONENT_SRCDIRS that would otherwise be compiled." >> ${output_mk}
    echo -e "COMPONENT_OBJS :=\n" >> ${output_mk}

    echo "###" >> ${output_mk}
    echo "# Directory paths, must be relative to the component directory, which will be searched for source files (*.cpp, *.c, *.S)." >> ${output_mk}
    echo "# Defaults to '.', ie the component directory itself." >> ${output_mk}
    echo "# Override this to specify a different list of directories which contain source files." >> ${output_mk}
    echo -e "COMPONENT_SRCDIRS +=\n" >> ${output_mk}

    echo "###" >> ${output_mk}
    echo "# set CFLAGS" >> ${output_mk}
    echo -e "CFLAGS +=\n" >> ${output_mk}

    echo "###" >> ${output_mk}
    echo "# set CXXFLAGS" >> ${output_mk}
    echo -e "CXXFLAGS +=\n" >> ${output_mk}

    echo "###" >> ${output_mk}
    echo "# set LDLAGS" >> ${output_mk}
    echo -e "LDLAGS +=\n" >> ${output_mk}
}

if [ ! -d ${module_name} ]; then
    mkdir ${module_name}
fi

cd ${module_name}

gen_kconfig
gen_mk

if [ ! -d include ]; then
    mkdir include
    echo -e "" > include/gitkeep
fi
