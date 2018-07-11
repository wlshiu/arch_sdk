#
# Copyright (c) 2018 Wei-Lun Hsu. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# @author  Wei-Lun Hsu
#

# Do not:
# o  use make's built-in rules and variables
#    (this increases performance and avoids hard-to-debug behaviour);
# o  print "Entering directory ...";
MAKEFLAGS += -rR --no-print-directory

# Avoid funny character set dependencies
unexport LC_ALL
LC_COLLATE=C
LC_NUMERIC=C
export LC_COLLATE LC_NUMERIC


KCONFIG_CONFIG := .config
export KCONFIG_CONFIG

# SHELL used by kbuild
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)


# $(warning tracing)
.PHONY: _all clean distclean help

###############
## check config file exsit or not
ifneq (,$(wildcard $(KCONFIG_CONFIG)))

# $(info PWD='$(CURDIR)')
include $(KCONFIG_CONFIG)

else

_all: no_config_error
.PHONY: no_config_error
no_config_error:
	@echo "No .config file. Please configure by 'make menuconfig'"
	@$(MAKE) -s UNKNOWN 2>/dev/null

endif


###############
## The all: target is the default when no target is given on the command line.
_all: all

###############
## Set the source tree
ifeq ($(KBUILD_SRC),)
    # building in the source tree
    srctree := .
else
    ifeq ($(KBUILD_SRC)/,$(dir $(CURDIR)))
        # building in a subdirectory of the source tree
        srctree := ..
    else
        srctree := $(KBUILD_SRC)
    endif
endif

objtree := .
src     := $(srctree)
obj     := $(objtree)

VPATH  := $(srctree)$(if $(KBUILD_EXTMOD),:$(KBUILD_EXTMOD))
TOPDIR := $(srctree)

export srctree objtree VPATH TOPDIR


###############
## Set verbose to make
Q := @
KBUILD_VERBOSE = 0
quiet=quiet_
ifeq ("$(origin V)", "command line")
    ifeq ($(V),1)
        Q =
        KBUILD_VERBOSE = 1
        quiet=
    endif
endif

# If the user is running make -s (silent mode), suppress echoing of
# commands

ifneq ($(filter 4.%,$(MAKE_VERSION)),)
    # make-4
    ifneq ($(filter %s ,$(firstword x$(MAKEFLAGS))),)
        quiet=silent_
    endif
else
    # make-3.8x
    ifneq ($(filter s% -s%,$(MAKEFLAGS)),)
        quiet=silent_
    endif
endif

export quiet Q KBUILD_VERBOSE

###############
## Set output directory
# OK, Make called in directory where kernel src resides
# Do we want to locate output files in a separate directory?
KBUILD_OUTPUT := output
ifeq ("$(origin O)", "command line")
    ifneq ("$(O)", "")
        KBUILD_OUTPUT := $(O)
    endif
endif

$(if $(findstring clean,$(MAKECMDGOALS)),,$(shell mkdir -p $(KBUILD_OUTPUT)))

###############
## Host compiler setting
HOSTCC       = gcc
HOSTCXX      = g++
HOSTCFLAGS   = -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89
HOSTCXXFLAGS = -O2

export HOSTCC HOSTCXX HOSTCXXFLAGS CONFIG_SHELL

# We need some generic definitions (do not try to remake the file).
scripts/Kbuild.include: ;
include scripts/Kbuild.include

# ===========================================================================
# Rules shared between *config targets and build targets

# Basic helpers built in scripts/
.PHONY: scripts_basic
scripts_basic:
	$(Q)$(MAKE) $(build)=scripts/basic
	$(Q)rm -f .tmp_quiet_recordmcount

# To avoid any implicit rule to kick in, define an empty command.
scripts/basic/%: scripts_basic ;

.PHONY: outputmakefile
# outputmakefile generates a Makefile in the output directory, if using a
# separate output directory. This allows convenient use of make in the
# output directory.
outputmakefile:
ifneq ($(KBUILD_SRC),)
	$(Q)ln -fsn $(srctree) source
	$(Q)$(CONFIG_SHELL) $(srctree)/scripts/mkmakefile \
	    $(srctree) $(objtree) $(VERSION) $(PATCHLEVEL)
endif

config-targets := 0
ifneq ($(filter config %config,$(MAKECMDGOALS)),)
    config-targets := 1
endif



ifeq ($(config-targets),1)
# ===========================================================================
# *config targets only - make sure prerequisites are updated, and descend
# in scripts/kconfig to make the *config target

###############
## include CPU setting by Makefile if you necessary
# -include $(srctree)/arch/$(SRCARCH)/Makefile
# export KBUILD_DEFCONFIG KBUILD_KCONFIG

.PHONY: config %config
config: scripts_basic outputmakefile
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

# %config: scripts_basic outputmakefile FORCE
%config: scripts_basic outputmakefile
	$(Q)$(MAKE) $(build)=scripts/kconfig $@


else

# Read in config
-include include/config/auto.conf

# Read in dependencies to all Kconfig* files, make sure to run
# oldconfig if changes are detected.
-include include/config/auto.conf.cmd

# To avoid any implicit rule to kick in, define an empty command
$(KCONFIG_CONFIG) include/config/auto.conf.cmd: ;

# If .config is newer than include/config/auto.conf, someone tinkered
# with it and forgot to run make oldconfig.
# if auto.conf.cmd is missing then we are probably in a cleaned tree so
# we execute the config step to be sure to catch updated Kconfig files
include/config/%.conf: $(KCONFIG_CONFIG) include/config/auto.conf.cmd
	$(Q)$(MAKE) -f $(srctree)/Makefile silentoldconfig


endif

###############
## the master TARGET
.PHONY: all
all:
	@echo "first target"
	@echo "do something"

###############
## help description
help:
	@echo  'Cleaning targets:'
	@echo  '  clean		  - Remove most generated files but keep the config and'
	@echo  '                    enough build support to build external modules'
	@echo  '  distclean	  - Remove all generated files + config + various backup files '
	@echo  '  					+ remove editor backup and patch files'
	@echo  ''
	@echo  '----------------------------------------------------------------------'
	@echo  '  make O=dir [targets] Locate all output files in "dir", including .config'
	@echo  '  		     default: output'
	@echo  '  make V=0|1 [targets] 0 => quiet build (default), 1 => verbose build'
	@echo  ''
	@echo  'not ready.......'

###############
## clean option
clean:
	@-rm -rf $(KBUILD_OUTPUT)


distclean:
	@-rm -rf $(KBUILD_OUTPUT)
	@-rm -f $(srctree)/scripts/basic/.*.cmd $(srctree)/scripts/basic/*.exe
	@-rm -f $(srctree)/scripts/kconfig/.*.cmd $(srctree)/scripts/kconfig/*.o $(srctree)/scripts/kconfig/*.exe
	@-rm -f $(KCONFIG_CONFIG) $(KCONFIG_CONFIG).old include/autoconf.h
	@-rm -fr $(srctree)/include/generated $(srctree)/include/config


