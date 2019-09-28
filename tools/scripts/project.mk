#
# Main Project Makefile
# This Makefile is included directly from the user project Makefile in order to call the component.mk
# makefiles of all components (in a separate make process) to build all the libraries, then links them
# together into the final file. If so, PWD is the project dir (we assume).
#

PHONY := build-components all build clean distclean info env_setup all_binaries
PHONY += menuconfig defconfig savedefconfig %_defconfig
PHONY += docs size tags TAGS cscope gtags toolchain toolchain-clean release

all: info env_setup all_binaries
# see below for recipe of 'all' target
#
# # other components will add dependencies to 'all_binaries'. The
# reason all_binaries is used instead of 'all' is so that the flash
# target can build everything without triggering the per-component "to
# flash..." output targets.)

help:
	@echo "----------------------------------------------------------------------"
	@echo "Welcome to build system. Some useful make targets:"
	@echo ""
	@echo "  make menuconfig            - Configure project"
	@echo "  make defconfig             - Set defaults for all new configuration options"
	@echo "  make xxx_defconfig         - Use default configuration options which is in $(srctree)/configs directory"
	@echo ""
	@echo "  make toolchain             - Prepare toolchain if you do not use host environment setting."
	@echo "                               ps. It should be executed before make compile"
	@echo "  make toolchain-clean       - Clean toolchain"
	@echo ""
	@echo "  make all                   - Build app, bootloader, partition table"
	@echo "  make clean                 - Remove most generated files but keep the config and"
	@echo "                               enough build support to build external modules"
	@echo "  make distclean             - Remove all generated files + config + various backup files"
	@echo ""
	@echo "  make release               - Pack SDK for release"
	@echo ""
	@echo "  make app                   - Build just the app"
	@echo "  make app-list              - List the executable app"
	@echo "  make app-clean             - Clean just the app"
	@echo ""
	@echo "  make list                  - List component libraries"
	@echo "  make list-config           - List config files in $(srctree)/configs"
	@echo ""
	@echo "Select unittest of apps"
	@echo "  make TEST_COMPONENTS=components-name - Build the unittest with the names of the components having \'test\' subdirectory"
	@echo "  make TESTS_ALL=1           - Build the unittest with all the tests for components having \'test\' subdirectory."
	@echo ""
	@echo "  make tags/TAGS             - Generate tags file for editors"
	@echo "  make cscope                - Generate cscope index"
	@echo "  make gtags                 - Generate GNU GLOBAL index"
	@echo ""
	@echo "  make astyle directory      - Format syntax with directory"
	@echo ""
	@echo "  make docs                  - Generate documentations"
	@echo ""
	@echo "Development functions"
	@echo "  make size                  - List section size infomation and output *.nm file"
	@echo "  make objdump               - Disassemble objects and output *.objdump file"
	@echo ""
	@echo "  make V=0|1 [targets] 0 => quiet build (default), 1 => verbose build"
	@echo "  make O=dir [targets] Locate all output files in 'dir', including autoconfig"
	@echo "----------------------------------------------------------------------"

# dependency checks
ifndef MAKE_RESTARTS
ifeq ("$(filter 4.% 3.81 3.82,$(MAKE_VERSION))","")
$(warning "Build system only supports GNU Make versions 3.81 or newer. You may see unexpected results with other Makes.")
endif
endif


#===========================================================================
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

cmd = @$(echo-cmd) $(cmd_$(1))

export CONFIG_SHELL cmd

# The directory where we put all objects/libraries/binaries. The project Makefile can
# configure this if needed.
BUILD_DIR_BASE ?= $(BUILD_OUTPUT)
export BUILD_DIR_BASE

# ---------------------------------------------------------------------------
# list apps
# ---------------------------------------------------------------------------
APPS := $(dir $(sort $(wildcard $(srctree)/apps/*/component.mk)))
APPS := $(notdir $(foreach app,$(APPS),$(lastword $(subst /, ,$(app)))))
export APPS

# ---------------------------------------------------------------------------
# list link scripts
# ---------------------------------------------------------------------------
LINK_SCRIPTS_DIRS := \
	$(srctree)/device \
	$(srctree)/apps

LINK_SCRIPTS := $(sort $(foreach devsrcdir,$(LINK_SCRIPTS_DIRS),$(wildcard $(devsrcdir)/*/*.ld)))
export LINK_SCRIPTS

# ---------------------------------------------------------------------------
# list tool chain
# ---------------------------------------------------------------------------
TOOLCHAIN_DIRS := \
	$(srctree)/tools/toolchain

TOOLCHAINS := $(sort $(foreach toolchainsrcdir,$(TOOLCHAIN_DIRS),$(wildcard $(toolchainsrcdir)/*.zip)))
TOOLCHAINS += $(sort $(foreach toolchainsrcdir,$(TOOLCHAIN_DIRS),$(wildcard $(toolchainsrcdir)/*.tar.bz2)))
export TOOLCHAINS

# ---------------------------------------------------------------------------
# list devices
# ---------------------------------------------------------------------------
DEVICE_DIRS := \
	$(srctree)/device

DEVICES := $(sort $(foreach devdir,$(DEVICE_DIRS),$(dir $(wildcard $(devdir)/*/Kconfig))))
DEVICES := $(notdir $(foreach devdir,$(DEVICES),$(lastword $(subst /, ,$(devdir)))))
export DEVICES

# ---------------------------------------------------------------------------
# list core images
# ---------------------------------------------------------------------------
IMAGE_DIRS := \
	$(srctree)/core_img

CORE_IMAGES := $(sort $(foreach imgdir,$(IMAGE_DIRS),$(wildcard $(imgdir)/*.bin)))
export CORE_IMAGES

# ---------------------------------------------------------------------------
# Set variables common to both project & component
# ---------------------------------------------------------------------------
include $(srctree)/tools/scripts/common.mk
PROJECT_NAME := $(call dequote,$(CONFIG_TARGET_APP_PROJECT))
PROJECT_PATH := $(addsuffix $(strip $(PROJECT_NAME)),$(srctree)/apps/)
export PROJECT_NAME PROJECT_PATH

# ---------------------------------------------------------------------------
# TODO: version
# Git version
# ---------------------------------------------------------------------------
GIT := $(shell command -v git 2> /dev/null)

ifdef GIT
GIT_COMMINT_VER := $(shell $(GIT) describe --long --all --always --abbrev=8 | sed 's/.*-g\(.*\)/\1/')
endif

KCONFIG_AUTO_FILES := \
	$(srctree)/device/Kconfig.linkscript \
	$(srctree)/device/Kconfig.devices \
	$(srctree)/core_img/Kconfig.imgs \
	$(srctree)/tools/toolchain/Kconfig.toolchain \
	$(srctree)/apps/Kconfig.app

export KCONFIG_AUTO_FILES

info:
	@if [ ! -z $(GIT) ]; then $(GIT) config --global core.autocrlf input; fi
	@echo -e $(YELLOW) "The Git commit SHA-1 ID \"$(GIT_COMMINT_VER)\"" $(NC)

env_setup:
	$(Q)if [ ! -f $(srctree)/device/Kconfig.linkscript ]; then echo "GEN    Link-Script Kconfig"; $(srctree)/tools/scripts/gen_ld_kconfig.sh $(srctree)/device $(LINK_SCRIPTS); fi
	$(Q)if [ ! -f $(srctree)/device/Kconfig.devices ]; then echo "GEN    Device Kconfig"; $(srctree)/tools/scripts/gen_device_kconfig.sh $(srctree)/device $(DEVICES); fi
	$(Q)if [ ! -f $(srctree)/core_img/Kconfig.imgs ]; then echo "GEN    Images Kconfig"; $(srctree)/tools/scripts/gen_img_kconfig.sh $(srctree)/core_img $(CORE_IMAGES); fi
	$(Q)if [ ! -f $(srctree)/apps/Kconfig.app ]; then echo "GEN    App Kconfig"; $(srctree)/tools/scripts/gen_app_kconfig.sh $(srctree)/apps $(APPS); fi
	$(Q)if [ ! -f $(srctree)/tools/toolchain/Kconfig.toolchain ]; then echo "GEN    Tool-chain Kconfig"; $(srctree)/tools/scripts/gen_toolchain_kconfig.sh $(srctree)/tools/toolchain $(TOOLCHAINS); fi
# ---------------------------------------------------------------------------
# astyle format syntax
# ---------------------------------------------------------------------------
ASTYLE_TOOL_DIR := $(srctree)/tools/astyle/build/gcc
ASTYLE := $(ASTYLE_TOOL_DIR)/bin/astyle
export ASTYLE_TOOL_DIR ASTYLE

astyle:
	@echo -e $(YELLOW) "Format Syntax..." $(NC)
	$(Q)if [ ! -f $(ASTYLE) ]; then echo "Build astyle"; CXX=$(HOSTCXX) CC=$(HOSTCC) LD=$(HOSTLD) CFLAGS= $(MAKE) -C $(ASTYLE_TOOL_DIR); fi
	@cd $(srctree)
	@echo -e $(RED)"Not yet" $(NC)

DOXYOBJ_FILES :=
DOXYGEN := @doxygen
docs: doxyfile.inc
	@echo -e $(YELLOW) "Running doxygen to create documentations" $(NC)
	@mkdir -p $(srctree)/doc/html
	$(DOXYGEN) $(srctree)/tools/scripts/doxyfile.mk > doxy.log 2>&1

export DOXYGEN DOXYOBJ_FILES

list-config:
	$(summary) $(YELLOW) "List configs in $(srctree)/configs" $(NC)
	@ls $(srctree)/configs/*_defconfig | sed 's:$(srctree)/configs/::g' | sort

#===========================================================================


# disable built-in make rules, makes debugging saner
MAKEFLAGS_OLD := $(MAKEFLAGS)
MAKEFLAGS +=-rR

# Default path to the project: we assume the Makefile including this file
# is in the project directory
ifndef PROJECT_PATH
PROJECT_PATH := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
export PROJECT_PATH
endif

# A list of the "common" makefiles, to use as a target dependency
COMMON_MAKEFILES := $(abspath $(srctree)/tools/scripts/project.mk $(srctree)/tools/scripts/common.mk $(srctree)/tools/scripts/component_wrapper.mk)
export COMMON_MAKEFILES

# Component directories. These directories are searched for components.
# The project Makefile can override these component dirs, or define extra component directories.
COMPONENT_DIRS ?= $(PROJECT_PATH)/libs \
				$(EXTRA_MODULE_DIRS) \
				$(srctree)/middleware/third_party \
				$(srctree)/middleware/prebuild

export COMPONENT_DIRS

# Source directories of the project itself (a special, project-specific component.) Defaults to only "main".
SRCDIRS ?= $(PROJECT_PATH)

# The project Makefile can define a list of components, but if it does not do this we just take
# all available components in the component dirs.
ifndef COMPONENTS
# Find all component names. The component names are the same as the
# directories they're in, so /bla/components/mycomponent/ -> mycomponent. We then use
# COMPONENT_DIRS to build COMPONENT_PATHS with the full path to each component.
COMPONENTS := $(foreach dir,$(COMPONENT_DIRS),$(wildcard $(dir)/*))
COMPONENTS := $(sort $(foreach comp,$(COMPONENTS),$(lastword $(subst /, ,$(comp)))))
endif
export COMPONENTS

# Resolve all of COMPONENTS into absolute paths in COMPONENT_PATHS.
#
# If a component name exists in multiple COMPONENT_DIRS, we take the first match.
#
# NOTE: These paths must be generated WITHOUT a trailing / so we
# can use $(notdir x) to get the component name.
COMPONENT_PATHS := $(foreach comp,$(COMPONENTS),$(firstword $(foreach dir,$(COMPONENT_DIRS),$(wildcard $(dir)/$(comp)))))
COMPONENT_PATHS += $(abspath $(SRCDIRS))

#===========================================================================
# ---------------------------------------------------------------------------
# filter components with CONFIG_ENABLE_xxx
# ---------------------------------------------------------------------------
DEVICE_COMPONENT_PATHS := $(foreach comp,$(DEVICES),$(firstword $(foreach dir,$(srctree)/device,$(wildcard $(dir)/$(comp)))))

CONFIG_ENABLE_COMPONENTS :=

define filterConfigComponents
ifeq ($$(CONFIG_ENABLE_$(1)),y)
$(3) += $(2)
endif
endef

$(foreach comp,$(COMPONENTS),$(eval $(call filterConfigComponents,$(shell echo $(subst .,_,$(comp)) | tr a-z A-Z),$(comp),CONFIG_ENABLE_COMPONENTS)))
$(foreach comp,$(DEVICES),$(eval $(call filterConfigComponents,$(shell echo $(subst .,_,$(comp)) | tr a-z A-Z),$(comp),CONFIG_ENABLE_COMPONENTS)))

# ---------------------------------------------------------------------------
# filter core images with CONFIG_ENABLE_xxx
# ---------------------------------------------------------------------------
IMG_FLAGS :=
SECTION_NAMES :=
$(eval IMG_NUM=$(shell echo $$(($(CONFIG_CORE_IMG_NUM)+1))))

define filterConfigImgs
ifeq ($$(CONFIG_INSERT_CORE_IMAGE_$(1)),y)
$(2) += --add-section $$(CONFIG_SECTION_NAME_$(1))=$(addprefix $(srctree)/core_img/,$$(CONFIG_IMAGE_NAME_$(1)))
$(3) += $$(CONFIG_SECTION_NAME_$(1))
endif
endef

$(foreach idx,$(shell seq 1 ${IMG_NUM}),$(eval $(call filterConfigImgs,$(idx),IMG_FLAGS,SECTION_NAMES)))
IMG_FLAGS := $(subst ",,$(IMG_FLAGS))

#===========================================================================

# A component is buildable if it has a component.mk makefile in it
CONFIG_ENABLE_COMPONENT_PATHS := $(foreach comp,$(CONFIG_ENABLE_COMPONENTS),$(firstword $(foreach dir,$(COMPONENT_DIRS),$(wildcard $(dir)/$(comp)))))
CONFIG_ENABLE_COMPONENT_PATHS += $(foreach comp,$(CONFIG_ENABLE_COMPONENTS),$(firstword $(foreach dir,$(srctree)/device,$(wildcard $(dir)/$(comp)))))
COMPONENT_PATHS_BUILDABLE := $(foreach cp,$(CONFIG_ENABLE_COMPONENT_PATHS),$(if $(wildcard $(cp)/component.mk),$(cp)))
COMPONENT_PATHS_BUILDABLE += $(SRCDIRS)

# If TESTS_ALL set to 1, set TEST_COMPONENTS to all components
ifeq ($(TESTS_ALL),1)
TEST_COMPONENTS := $(COMPONENTS)
endif

# If TEST_COMPONENTS is set, create variables for building unit tests
ifdef TEST_COMPONENTS
# TODO: unit-test
override TEST_COMPONENTS := $(foreach comp,$(TEST_COMPONENTS),$(wildcard $(COMPONENT_DIRS)/$(comp)/test))
TEST_COMPONENT_PATHS := $(TEST_COMPONENTS)
TEST_COMPONENT_NAMES :=  $(foreach comp,$(TEST_COMPONENTS),$(lastword $(subst /, ,$(dir $(comp))))_test)
endif

# Initialise project-wide variables which can be added to by
# each component.
#
# These variables are built up via the component_project_vars.mk
# generated makefiles (one per component).
#
# See docs/build-system.rst for more details.
COMPONENT_INCLUDES :=
COMPONENT_LDFLAGS :=
COMPONENT_SUBMODULES :=

# COMPONENT_PROJECT_VARS is the list of component_project_vars.mk generated makefiles
# for each component.
#
# Including $(COMPONENT_PROJECT_VARS) builds the COMPONENT_INCLUDES,
# COMPONENT_LDFLAGS variables and also targets for any inter-component
# dependencies.
#
# See the component_project_vars.mk target in component_wrapper.mk
COMPONENT_PROJECT_VARS := $(addsuffix /component_project_vars.mk,$(notdir $(COMPONENT_PATHS_BUILDABLE) ) $(TEST_COMPONENT_NAMES))
COMPONENT_PROJECT_VARS := $(addprefix $(BUILD_DIR_BASE)/,$(COMPONENT_PROJECT_VARS))
# this line is -include instead of include to prevent a spurious error message on make 3.81
-include $(COMPONENT_PROJECT_VARS)

# Also add top-level project include path, for top-level includes
COMPONENT_INCLUDES += $(abspath $(BUILD_DIR_BASE)/include/)

export COMPONENT_INCLUDES

TOOLCHAIN_PATH :=
ifneq ($(CONFIG_TARGET_TOOLCHAIN_PATH), "")
ifneq ("$(wildcard $(srctree)/tools/toolchain/active)","")
TOOLCHAIN_PATH := $(shell find $(srctree)/tools/toolchain/active -type f -name 'arm-none-eabi-objcopy*' -print | grep 'bin' | xargs dirname)
TOOLCHAIN_PATH := $(addsuffix /,$(TOOLCHAIN_PATH))
endif
endif
export TOOLCHAIN_PATH

all:
	@echo -e $(YELLOW) "Done..."$(NC)

toolchain:
	$(Q)if [ ! -z $(CONFIG_TARGET_TOOLCHAIN_PATH) ] && [ ! -d $(srctree)/tools/toolchain/active ]; then \
		echo -e "Un-tar toolchain ... $(CONFIG_TARGET_TOOLCHAIN_PATH)" ; \
		mkdir $(srctree)/tools/toolchain/active ; \
		$(srctree)/tools/scripts/untar_toolchain.sh $(CONFIG_TARGET_TOOLCHAIN_PATH) $(srctree)/tools/toolchain/active ; \
	fi;

toolchain-clean:
	@echo -e "remove active toolchain"
	@rm -fr $(srctree)/tools/toolchain/active

# Set CPU options
CPU_FLAGS := -marm -mlittle-endian -mthumb -mcpu=cortex-m4 -march=armv7e-m

# Set default LDFLAGS
LDFLAGS ?= \
	$(addprefix -L$(BUILD_DIR_BASE)/,$(COMPONENTS) $(DEVICES) $(TEST_COMPONENT_NAMES)) \
	$(addprefix -L$(BUILD_DIR_BASE)/,$(PROJECT_NAME) ) \
	$(EXTRA_LDFLAGS) \
	-specs=nosys.specs \
	-Wl,--gc-sections	\
	-Wl,-static	\
	-Wl,--start-group	\
	$(COMPONENT_LDFLAGS) \
	-Wl,--end-group \
	-Wl,-EL

LDFLAGS += -lc -lm -lnosys
LDFLAGS += -T$(CONFIG_TARGET_LD_FILE)
LDFLAGS += -L$(srctree)/middleware/prebuild


# LDFLAGS += -nostdlib -lstdc++ -lgcc
# LDFLAGS += -u call_user_start_cpu0

# Set default CPPFLAGS, CFLAGS, CXXFLAGS
# These are exported so that components can use them when compiling.
# If you need your component to add CFLAGS/etc for it's own source compilation only, set CFLAGS += in your component's Makefile.
# If you need your component to add CFLAGS/etc globally for all source
#  files, set CFLAGS += in your component's Makefile.projbuild
# If you need to set CFLAGS/CPPFLAGS/CXXFLAGS at project level, set them in application Makefile
#  before including project.mk. Default flags will be added before the ones provided in application Makefile.

# CPPFLAGS used by C preprocessor
# If any flags are defined in application Makefile, add them at the end.
CPPFLAGS := -MMD -MP $(CPPFLAGS) $(EXTRA_CPPFLAGS)

# Warnings-related flags relevant both for C and C++
COMMON_WARNING_FLAGS = -Wall -Werror=all \
	-Wno-error=unused-function \
	-Wno-error=unused-but-set-variable \
	-Wno-error=unused-variable \
	-Wno-error=deprecated-declarations \
	-Wextra \
	-Wno-unused-parameter -Wno-sign-compare

# Flags which control code generation and dependency generation, both for C and C++
COMMON_FLAGS = \
	$(CPU_FLAGS) \
	-include $(BUILD_DIR_BASE)/include/autoconfig.h \
	-ffunction-sections -fdata-sections \
	-fstrict-volatile-bitfields

# COMMON_FLAGS += -nostdlib
# COMMON_FLAGS += -mlongcalls

# Optimization flags are set based on menuconfig choice
ifneq ("$(CONFIG_OPTIMIZATION_LEVEL_RELEASE)","")
OPTIMIZATION_FLAGS = -Os
CPPFLAGS += -DNDEBUG
else
OPTIMIZATION_FLAGS = -Og
endif

# Enable generation of debugging symbols
OPTIMIZATION_FLAGS += -ggdb

# List of flags to pass to C compiler
# If any flags are defined in application Makefile, add them at the end.
CFLAGS := $(strip \
	-std=gnu99 \
	$(OPTIMIZATION_FLAGS) \
	$(COMMON_FLAGS) \
	$(COMMON_WARNING_FLAGS) -Wno-old-style-declaration \
	$(CFLAGS) \
	$(EXTRA_CFLAGS))


PREBUILD_INC=$(shell find $(srctree)/middleware/prebuild/ -type d -name 'include')
CFLAGS += $(foreach incpath,$(PREBUILD_INC),$(addprefix -I ,$(incpath))) \
	-I $(srctree)/middleware/prebuild

# List of flags to pass to C++ compiler
# If any flags are defined in application Makefile, add them at the end.
CXXFLAGS := $(strip \
	-std=gnu++11 \
	-fno-exceptions \
	-fno-rtti \
	$(OPTIMIZATION_FLAGS) \
	$(COMMON_FLAGS) \
	$(COMMON_WARNING_FLAGS) \
	$(CXXFLAGS) \
	$(EXTRA_CXXFLAGS))

OBJCOPY_FLAGS := -O binary

export CFLAGS CPPFLAGS CXXFLAGS OBJCOPY_FLAGS

# Set host compiler and binutils
HOSTCC := $(CC)
HOSTCXX := g++
HOSTLD := $(LD)
HOSTAR := $(AR)
HOSTOBJCOPY := $(OBJCOPY)
HOSTSIZE := $(SIZE)
export HOSTCC HOSTLD HOSTAR HOSTOBJCOPY SIZE

# Set target compiler. Defaults to whatever the user has
# configured as prefix + ye olde gcc commands
CC := $(call dequote,$(CONFIG_TOOLPREFIX))gcc
CXX := $(call dequote,$(CONFIG_TOOLPREFIX))c++
LD := $(call dequote,$(CONFIG_TOOLPREFIX))ld
AR := $(call dequote,$(CONFIG_TOOLPREFIX))ar
OBJCOPY := $(call dequote,$(CONFIG_TOOLPREFIX))objcopy
OBJDUMP := $(call dequote,$(CONFIG_TOOLPREFIX))objdump
SIZE := $(call dequote,$(CONFIG_TOOLPREFIX))size
NM := $(call dequote,$(CONFIG_TOOLPREFIX))nm

CC := $(TOOLCHAIN_PATH)$(CC)
CXX := $(TOOLCHAIN_PATH)$(CXX)
LD := $(TOOLCHAIN_PATH)$(LD)
AR := $(TOOLCHAIN_PATH)$(AR)
OBJCOPY := $(TOOLCHAIN_PATH)$(OBJCOPY)
OBJDUMP := $(TOOLCHAIN_PATH)$(OBJDUMP)
SIZE := $(TOOLCHAIN_PATH)$(SIZE)
NM := $(TOOLCHAIN_PATH)$(NM)
export CC CXX LD AR OBJCOPY SIZE OBJDUMP NM

# TODO: python
# PYTHON=$(call dequote,$(CONFIG_PYTHON))

# the app is the main executable built by the project
APP_ELF_ORG:=$(BUILD_DIR_BASE)/$(PROJECT_NAME).elf.org
APP_ELF:=$(BUILD_DIR_BASE)/$(PROJECT_NAME).elf
APP_MAP:=$(APP_ELF:.elf=.map)
APP_BIN:=$(APP_ELF:.elf=.bin)
APP_SYMBOL:=$(APP_ELF:.elf=.symbol)
APP_OBJDUMP:=$(APP_ELF:.elf=.objdump)

# Include any Makefile.projbuild file letting components add
# configuration at the project level
define includeProjBuildMakefile
$(if $(V),$(if $(wildcard $(1)/Makefile.projbuild),$(info including $(1)/Makefile.projbuild...)))
COMPONENT_PATH := $(1)
-include $(1)/Makefile.projbuild
endef
$(foreach componentpath,$(COMPONENT_PATHS),$(eval $(call includeProjBuildMakefile,$(componentpath))))

# once we know component paths, we can include the config generation targets
#
# (bootloader build doesn't need this, config is exported from top-level)
ifndef IS_BOOTLOADER_BUILD
include $(srctree)/tools/scripts/project_config.mk
endif

# A "component" library is any library in the LDFLAGS where
# the name of the library is also a name of the component
APP_LIBRARIES = $(patsubst -l%,%,$(filter -l%,$(LDFLAGS)))
COMPONENT_LIBRARIES = $(filter $(notdir $(COMPONENT_PATHS_BUILDABLE)) $(TEST_COMPONENT_NAMES),$(APP_LIBRARIES))

# ELF depends on the library archive files for COMPONENT_LIBRARIES
# the rules to build these are emitted as part of GenerateComponentTarget below
#
# also depends on additional dependencies (linker scripts & binary libraries)
# stored in COMPONENT_LINKER_DEPS, built via component.mk files' COMPONENT_ADD_LINKER_DEPS variable
$(APP_ELF_ORG): $(foreach libcomp,$(COMPONENT_LIBRARIES),$(BUILD_DIR_BASE)/$(libcomp)/lib$(libcomp).a) $(COMPONENT_LINKER_DEPS)
	$(summary) $(GREEN)  LD $(notdir $@) $(NC)
	$(CC) $(LDFLAGS) -o $@ -Wl,-Map=$(APP_MAP)

$(APP_BIN): $(APP_ELF_ORG)
	$(summary) $(YELLOW) "Post Build Steps ................."$(NC)
	@$(OBJCOPY) $(IMG_FLAGS) $(APP_ELF_ORG) $(APP_ELF)
	$(summary) $(GREEN) " insert section: $(SECTION_NAMES)"$(NC)
	@$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@


doxyfile.inc: $(foreach libcomp,$(COMPONENT_LIBRARIES),$(BUILD_DIR_BASE)/$(libcomp)/$(libcomp)-doxyobj)
	echo INPUT = $(COMPONENT_DIRS) > doxyfile.inc
	echo OUTPUT_DIRECTORY = $(srctree)/doc/html >> doxyfile.inc
	echo HTML_OUTPUT = $(srctree)/doc/html >> doxyfile.inc
	echo FILE_PATTERNS = *.h *.c $(DOXYOBJ_FILES) >> doxyfile.inc

# Generation of $(APP_BIN) from $(APP_ELF) is added by the esptool
# component's Makefile.projbuild
app: $(APP_BIN)
ifeq ("$(CONFIG_SECURE_BOOT_ENABLED)$(CONFIG_SECURE_BOOT_BUILD_SIGNED_BINARIES)","y") # secure boot enabled, but remote sign app image
	@echo "App built but not signed. Signing step via espsecure.py:"
	@echo "espsecure.py sign_data --keyfile KEYFILE $(APP_BIN)"
	@echo "Then flash app command is:"
	@echo $(ESPTOOLPY_WRITE_FLASH) $(CONFIG_APP_OFFSET) $(APP_BIN)
else
	@echo "App built. Default flash app command is:"
	@echo $(ESPTOOLPY_WRITE_FLASH) $(CONFIG_APP_OFFSET) $(APP_BIN)
endif

all_binaries: toolchain $(APP_BIN)


$(BUILD_DIR_BASE):
	mkdir -p $(BUILD_DIR_BASE)

# Macro for the recursive sub-make for each component
# $(1) - component directory
# $(2) - component name only
#
# Is recursively expanded by the GenerateComponentTargets macro
define ComponentMake
+$(MAKE) -C $(BUILD_DIR_BASE)/$(2) -f $(srctree)/tools/scripts/component_wrapper.mk COMPONENT_MAKEFILE=$(1)/component.mk COMPONENT_NAME=$(2)
endef

# Generate top-level component-specific targets for each component
# $(1) - path to component dir
# $(2) - name of component
#
define GenerateComponentTargets
PHONY += $(2)-build $(2)-clean $(2)-doxyobj

$(2)-build:
	$(summary) $(YELLOW) build $(2) $(NC)
	$(call ComponentMake,$(1),$(2)) build

$(2)-clean:
	$(summary) $(GREEN) "clean $(2)" $(NC)
	$(call ComponentMake,$(1),$(2)) clean

$(BUILD_DIR_BASE)/$(2)/$(2)-doxyobj:
	$(summary) $(GREEN) "Gen $(2) doc" $(NC)
	$(call ComponentMake,$(1),$(2)) doxyobj

$(BUILD_DIR_BASE)/$(2):
	@mkdir -p $(BUILD_DIR_BASE)/$(2)

# tell make it can build any component's library by invoking the -build target
# (this target exists for all components even ones which don't build libraries, but it's
# only invoked for the targets whose libraries appear in COMPONENT_LIBRARIES and hence the
# APP_ELF dependencies.)
$(BUILD_DIR_BASE)/$(2)/lib$(2).a: $(2)-build
	$(details) "Target '$$^' responsible for '$$@'" # echo which build target built this file

# add a target to generate the component_project_vars.mk files that
# are used to inject variables into project make pass (see matching
# component_project_vars.mk target in component_wrapper.mk).
#
# If any component_project_vars.mk file is out of date, the make
# process will call this target to rebuild it and then restart.
#
$(BUILD_DIR_BASE)/$(2)/component_project_vars.mk: $(1)/component.mk $(COMMON_MAKEFILES) $(SDKCONFIG_MAKEFILE) | $(BUILD_DIR_BASE)/$(2)
	$(call ComponentMake,$(1),$(2)) component_project_vars.mk
endef

$(foreach component,$(COMPONENT_PATHS_BUILDABLE),$(eval $(call GenerateComponentTargets,$(component),$(notdir $(component)))))
$(foreach component,$(TEST_COMPONENT_PATHS),$(eval $(call GenerateComponentTargets,$(component),$(lastword $(subst /, ,$(dir $(component))))_test)))

app-clean: $(addsuffix -clean,$(notdir $(COMPONENT_PATHS_BUILDABLE)))
	$(summary) RM $(APP_ELF)
	rm -f $(APP_ELF) $(APP_ELF_ORG) $(APP_BIN) $(APP_MAP) $(APP_SYMBOL) $(APP_OBJDUMP)

#===========================================================================
# ---------------------------------------------------------------------------
# Generate tags for editors
# ---------------------------------------------------------------------------
quiet_cmd_tags = GEN     $@
      cmd_tags = $(CONFIG_SHELL) $(srctree)/tools/scripts/tags.sh $@

tags TAGS cscope gtags:
	$(call cmd,tags)

# ---------------------------------------------------------------------------
# list executable apps
# ---------------------------------------------------------------------------
app-list:
	@echo $(APPS)

list:
	@echo $(COMPONENT_LIBRARIES) | sed 's/\s\+/\n/g'

# ---------------------------------------------------------------------------
# development functions
# ---------------------------------------------------------------------------
size: toolchain $(APP_BIN)
	@echo ""
	@echo -e $(YELLOW) "Size information"$(NC)
	@echo -e $(YELLOW) " Log to $(APP_SYMBOL)"$(NC)
	$(SIZE) -At -x $(APP_ELF)
	$(SIZE) -At -x $(APP_ELF) > $(APP_SYMBOL)
	@echo -e "\nSYMBOLS:\n" >> $(APP_SYMBOL)
	$(NM) -C -nslS -f bsd -t x $(APP_ELF) >> $(APP_SYMBOL)

objdump: toolchain $(APP_BIN)
	@echo ""
	@echo -e $(YELLOW) "Objects Dump to $(APP_OBJDUMP)"$(NC)
	$(OBJDUMP) -Sx $(APP_ELF) > $(APP_OBJDUMP)

# ---------------------------------------------------------------------------
# pack SDK for release
# ---------------------------------------------------------------------------
release:
	@echo -e $(RED)"new target 'release' (not yet): $(@)" $(NC)

#===========================================================================

# NB: this ordering is deliberate (app-clean before config-clean),
# so config remains valid during all component clean targets
config-clean: app-clean
clean: config-clean
distclean: clean

.PHONY: $(PHONY)
