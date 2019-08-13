#
# Main Project Makefile
# This Makefile is included directly from the user project Makefile in order to call the module.mk
# makefiles of all modules (in a separate make process) to build all the libraries, then links them
# together into the final file. If so, PWD is the project dir (we assume).
#


.PHONY: build-modules menuconfig defconfig all build clean all_binaries tags TAGS cscope gtags
all: all_binaries # other modules will add dependencies to 'all_binaries'

# (the reason all_binaries is used instead of 'all' is so that the flash target
# can build everything without triggering the per-module "to flash..."
# output targets.)

help:
	@echo "Welcome to VT build system. Some useful make targets:"
	@echo ""
	@echo "make menuconfig - Configure IDF project"
	@echo "make defconfig - Set defaults for all new configuration options"
	@echo ""
	@echo "make all - Build app, bootloader, partition table"
	@echo "make clean - Remove all build output"
	@echo ""
	@echo "make app - Build just the app"
	@echo "make app-clean - Clean just the app"
	@echo ""


# disable built-in make rules, makes debugging saner
MAKEFLAGS +=-rR

# Figure out PROJECT_PATH if not set
ifeq ("$(PROJECT_PATH)","")
# The path to the project: we assume the Makefile including this file resides
# in the root of that directory.
PROJECT_PATH := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
export PROJECT_PATH
endif

############################################################
$(warning PROJECT_PATH=$(PROJECT_PATH))
############################################################

# The directory where we put all objects/libraries/binaries. The project Makefile can
# configure this if needed.
## BUILD_DIR_BASE ?= $(PROJECT_PATH)/build
BUILD_DIR_BASE ?= $(BUILD_PATH)


# Module directories. These directories are searched for modules.
# The project Makefile can override these module dirs, or define extra module directories.
MODULE_DIRS ?= $(PROJECT_PATH)/apps/modules \
				$(EXTRA_MODULE_DIRS) \
				$(srctree)/middleware/third_party \
				$(srctree)/middleware/vango \
				$(srctree)/middleware/prebuild

export MODULE_DIRS

# The project Makefile can define a list of modules, but if it does not do this we just take
# all available modules in the module dirs.
ifeq ("$(MODULES)","")
# Find all module names. The module names are the same as the
# directories they're in, so /bla/modules/mymodule/ -> mymodule. We later use
# the MODULE_DIRS bit to find back the module path.
MODULES := $(foreach dir,$(MODULE_DIRS),$(wildcard $(dir)/*))
MODULES := $(sort $(foreach comp,$(MODULES),$(lastword $(subst /, ,$(comp)))))
endif
export MODULES


APP_LIST := $(sort $(wildcard $(srctree)/apps/*/Makefile))
$(warning applist=$(APP_LIST))
# Sources default to only "main"
SRCDIRS ?= main

# Here, we resolve and add all the modules and source paths into absolute paths.
# If a module exists in multiple MODULE_DIRS, we take the first match.
# WARNING: These directories paths must be generated WITHOUT a trailing / so we
# can use $(notdir x) to get the module name.
MODULES_PATHS := $(foreach comp,$(MODULES),$(firstword $(foreach dir,$(MODULE_DIRS),$(wildcard $(dir)/$(comp)))))
MODULES_PATHS += $(abspath $(SRCDIRS))

# A module is buildable if it has a module.mk makefile; we assume that a
# 'make -C $(module dir) -f module.mk build' results in a lib$(modulename).a
MODULES_PATHS_BUILDABLE := $(foreach cp,$(MODULES_PATHS),$(if $(wildcard $(cp)/module.mk),$(cp)))

# Assemble global list of include dirs (MODULES_INCLUDES), and
# LDFLAGS args (MODULES_LDFLAGS) supplied by each module.
MODULES_INCLUDES :=
MODULES_LDFLAGS :=
#
# Also add any inter-module dependencies for each module.

# Extract a variable from a child make process
#
# $(1) - path to directory to invoke make in
# $(2) - name of variable to print via the get_variable target (passed in GET_VARIABLE)
#
# needs 'sed' processing of stdout because make sometimes echoes other stuff on stdout,
# even if asked not to.
#
# Debugging this? Replace $(shell with $(error and you'll see the full command as-run.
define GetVariable
$(shell "$(MAKE)" -s --no-print-directory -C $(1) -f module.mk get_variable PROJECT_PATH=$(PROJECT_PATH) GET_VARIABLE=$(2) | sed -En "s/^$(2)=(.+)/\1/p" )
endef

MODULES_INCLUDES := $(abspath $(foreach comp,$(MODULES_PATHS_BUILDABLE),$(addprefix $(comp)/, \
	$(call GetVariable,$(comp),COMPONENT_ADD_INCLUDEDIRS))))

#Also add project include path, for sdk includes
MODULES_INCLUDES += $(BUILD_PATH)/include/
export MODULES_INCLUDES

#MODULES_LDFLAGS has a list of all flags that are needed to link the modules together. It's collected
#in the same way as MODULES_INCLUDES is.
MODULES_LDFLAGS := $(foreach comp,$(MODULES_PATHS_BUILDABLE), \
	$(call GetVariable,$(comp),MODULE_ADD_LDFLAGS))
export MODULES_LDFLAGS

# Generate module dependency targets from dependencies lists
# each module gains a target of its own <name>-build with dependencies
# of the names of any other modules (-build) that need building first
#
# the actual targets (that invoke submakes) are generated below by
# GenerateComponentTarget macro.
define GenerateModuleDependencies
# $(1) = module path
.PHONY: $$(notdir $(1))
$$(notdir $(1))-build: $(addsuffix -build,$(call GetVariable,$(1),MODULE_DEPENDS))
endef
$(foreach comp,$(MODULES_PATHS_BUILDABLE), $(eval $(call GenerateModuleDependencies,$(comp))))

#Make sure submakes can also use this.
export PROJECT_PATH

#Include functionality common to both project & module
-include $(srctree)/tools/scripts/common.mk

# Set default LDFLAGS

LDFLAGS ?= -nostdlib \
	-L$(srctree)/lib \
	-L$(srctree)/ld \
	$(addprefix -L$(BUILD_DIR_BASE)/,$(MODULES) $(SRCDIRS)) \
	-u call_user_start_cpu0	\
	-Wl,--gc-sections	\
	-Wl,-static	\
	-Wl,--start-group	\
	$(MODULES_LDFLAGS) \
	-lgcc \
	-Wl,--end-group \
	-Wl,-EL

# Set default CPPFLAGS, CFLAGS, CXXFLAGS
#
# These are exported so that modules can use them when compiling.
#
# If you need your module to add CFLAGS/etc for it's own source compilation only, set CFLAGS += in your module's Makefile.
#
# If you need your module to add CFLAGS/etc globally for all source
# files, set CFLAGS += in your module's Makefile.projbuild

# CPPFLAGS used by an compile pass that uses the C preprocessor
CPPFLAGS = -DESP_PLATFORM -Og -g3 -Wpointer-arith -Werror -Wno-error=unused-function -Wno-error=unused-but-set-variable \
		-Wno-error=unused-variable -Wall -ffunction-sections -fdata-sections -mlongcalls -nostdlib -MMD -MP

# C flags use by C only
CFLAGS = $(CPPFLAGS) -std=gnu99 -g3 -fstrict-volatile-bitfields

# CXXFLAGS uses by C++ only
CXXFLAGS = $(CPPFLAGS) -Og -std=gnu++11 -g3 -fno-exceptions -fstrict-volatile-bitfields

export CFLAGS CPPFLAGS CXXFLAGS

#Set host compiler and binutils
HOSTCC := $(CC)
HOSTLD := $(LD)
HOSTAR := $(AR)
HOSTOBJCOPY := $(OBJCOPY)

#Set target compiler. Defaults to whatever the user has
#configured as prefix + yer olde gcc commands
CC := $(call dequote,$(CONFIG_TOOLPREFIX))gcc
CXX := $(call dequote,$(CONFIG_TOOLPREFIX))c++
LD := $(call dequote,$(CONFIG_TOOLPREFIX))ld
AR := $(call dequote,$(CONFIG_TOOLPREFIX))ar
OBJCOPY := $(call dequote,$(CONFIG_TOOLPREFIX))objcopy
export CC CXX LD AR OBJCOPY

PYTHON=$(call dequote,$(CONFIG_PYTHON))

# the app is the main executable built by the project
APP_ELF:=$(BUILD_DIR_BASE)/$(PROJECT_NAME).elf
APP_MAP:=$(APP_ELF:.elf=.map)
APP_BIN:=$(APP_ELF:.elf=.bin)

# Include any Makefile.projbuild file letting modules add
# configuration at the project level
define includeProjBuildMakefile
$(if $(V),$(if $(wildcard $(1)/Makefile.projbuild),$(info including $(1)/Makefile.projbuild...)))
COMPONENT_PATH := $(1)
-include $(1)/Makefile.projbuild
endef
$(foreach modulepath,$(MODULES_PATHS),$(eval $(call includeProjBuildMakefile,$(modulepath))))

# once we know module paths, we can include the config
include $(srctree)/tools/scripts/project_config.mk

# A "module" library is any library in the LDFLAGS where
# the name of the library is also a name of the module
APP_LIBRARIES = $(patsubst -l%,%,$(filter -l%,$(LDFLAGS)))
MODULES_LIBRARIES = $(filter $(notdir $(MODULES_PATHS_BUILDABLE)),$(APP_LIBRARIES))


# SHELL used by kbuild
# ---------------------------------------------------------------------------
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

export CONFIG_SHELL

# Generate tags for editors
# ---------------------------------------------------------------------------
cmd = @$(echo-cmd) $(cmd_$(1))
quiet_cmd_tags = GEN     $@
      cmd_tags = $(CONFIG_SHELL) $(srctree)/tools/scripts/tags.sh $@

tags TAGS cscope gtags:
	$(call cmd,tags)

# ELF depends on the library archive files for MODULES_LIBRARIES
# the rules to build these are emitted as part of GenerateComponentTarget below
$(APP_ELF): $(foreach libmodule,$(MODULES_LIBRARIES),$(BUILD_DIR_BASE)/$(libmodule)/lib$(libmodule).a)
	$(summary) LD $(notdir $@)
	$(Q) $(CC) $(LDFLAGS) -o $@ -Wl,-Map=$(APP_MAP)

# Generation of $(APP_BIN) from $(APP_ELF) is added by the esptool
# module's Makefile.projbuild
app: $(APP_BIN)
	@echo "App built. Default flash app command is:"
	@echo $(ESPTOOLPY_WRITE_FLASH) $(CONFIG_APP_OFFSET) $(APP_BIN) #########################################

all_binaries: $(APP_BIN)

$(BUILD_DIR_BASE):
	mkdir -p $(BUILD_DIR_BASE)

define GenerateModulePhonyTarget
# $(1) - path to module dir
# $(2) - target to generate (build, clean)
.PHONY: $(notdir $(1))-$(2)
$(notdir $(1))-$(2): | $(BUILD_DIR_BASE)/$(notdir $(1))
	@+$(MAKE) -C $(BUILD_DIR_BASE)/$(notdir $(1)) -f $(1)/module.mk COMPONENT_BUILD_DIR=$(BUILD_DIR_BASE)/$(notdir $(1)) $(2)
endef

define GenerateModuleTargets
# $(1) - path to module dir
$(BUILD_DIR_BASE)/$(notdir $(1)):
	@mkdir -p $(BUILD_DIR_BASE)/$(notdir $(1))

# tell make it can build any module's library by invoking the recursive -build target
# (this target exists for all modules even ones which don't build libraries, but it's
# only invoked for the targets whose libraries appear in MODULES_LIBRARIES and hence the
# APP_ELF dependencies.)
$(BUILD_DIR_BASE)/$(notdir $(1))/lib$(notdir $(1)).a: $(notdir $(1))-build
	$(details) "Target '$$^' responsible for '$$@'" # echo which build target built this file
endef

$(foreach module,$(MODULES_PATHS_BUILDABLE),$(eval $(call GenerateModuleTargets,$(module))))

$(foreach module,$(MODULES_PATHS_BUILDABLE),$(eval $(call GenerateModulePhonyTarget,$(module),build)))
$(foreach module,$(MODULES_PATHS_BUILDABLE),$(eval $(call GenerateModulePhonyTarget,$(module),clean)))

app-clean: $(addsuffix -clean,$(notdir $(MODULES_PATHS_BUILDABLE)))
	$(summary) RM $(APP_ELF)
	$(Q) rm -f $(APP_ELF) $(APP_BIN) $(APP_MAP)

clean: app-clean


