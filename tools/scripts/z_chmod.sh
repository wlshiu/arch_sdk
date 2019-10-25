#!/bin/bash

set -e

# you should execute at src-root

ignore="-not -path './tools/toolchain/* -not -path './out/*' -not -path './.repo/*' -not -path './.git/*'"

find . -type f -name '*.c' ${ignore} -exec chmod -x {} \;
# find . -type f -name '*.cpp' ${ignore} -exec chmod -x {} \;
find . -type f -name '*.h' ${ignore} -exec chmod -x {} \;
find . -type f -name '*.ld' ${ignore} -exec chmod -x {} \;
find . -type f -name '*.s' ${ignore} -exec chmod -x {} \;
find . -type f -name '*.S' ${ignore} -exec chmod -x {} \;
find . -type f -name '*.md' ${ignore} -exec chmod -x {} \;
find . -type f -name '*.mk' ${ignore} -exec chmod -x {} \;
find . -type f -name '*.gdb' ${ignore} -exec chmod -x {} \;
find . -type f -name '*defconfig' ${ignore} -exec chmod -x {} \;
find . -type f -name 'Kconfig*' ${ignore} -exec chmod -x {} \;
find . -type f -name '.gitignore' ${ignore} -exec chmod -x {} \;
find . -type f -name 'Makefile' ${ignore} -exec chmod -x {} \;
find . -type f -name 'README*' ${ignore} -exec chmod -x {} \;

find . -type f -name '*.sh' ${ignore} -exec chmod +x {} \;
