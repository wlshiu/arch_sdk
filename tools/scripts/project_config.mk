# Makefile support for the menuconfig system

#Find all Kconfig files for all components
COMPONENT_KCONFIGS := $(foreach component,$(COMPONENT_PATHS),$(wildcard $(component)/Kconfig))
COMPONENT_KCONFIGS_PROJBUILD := $(foreach component,$(COMPONENT_PATHS),$(wildcard $(component)/Kconfig.projbuild))

COMPONENT_KCONFIGS := $(filter-out $(srctree)/apps/%,$(COMPONENT_KCONFIGS))

#For doing make menuconfig etc
KCONFIG_TOOL_DIR=$(srctree)/tools/scripts/kconfig

# set SDKCONFIG to the project's autoconfig,
# unless it's overriden (happens for bootloader)
SDKCONFIG ?= $(BUILD_OUTPUT)/autoconfig

# SDKCONFIG_DEFAULTS is an optional file containing default
# overrides (usually used for esp-idf examples)
## TODO: default config
SDKCONFIG_DEFAULTS ?= $(srctree)/apps/autoconfig.defaults

# reset MAKEFLAGS as the menuconfig makefile uses implicit compile rules
$(KCONFIG_TOOL_DIR)/mconf $(KCONFIG_TOOL_DIR)/conf:
	MAKEFLAGS=$(ORIGINAL_MAKEFLAGS) CC=$(HOSTCC) LD=$(HOSTLD) \
	$(MAKE) -C $(KCONFIG_TOOL_DIR)

#-----------------------------------------------------------------
PHONY := config-clean distclean defconfig menuconfig savedefconfig %_defconfig

ifeq ("$(wildcard $(SDKCONFIG))","")

ifneq ("$(filter %_defconfig, $(MAKECMDGOALS))","")
# user defconfig
SDKCONFIG := $(filter %_defconfig, $(MAKECMDGOALS))
# XCONFIG := $(filter %_defconfig, $(MAKECMDGOALS))
# $(XCONFIG): $(SDKCONFIG)
$(SDKCONFIG): env_setup %_defconfig

else ifeq ("$(filter defconfig, $(MAKECMDGOALS))","")
# if no configuration file is present and defconfig is not a named
# target, run defconfig then menuconfig to get the initial config
$(SDKCONFIG): menuconfig
menuconfig: defconfig

else 	# ifeq ("$(filter defconfig, $(MAKECMDGOALS))","")
# otherwise, just run defconfig
$(SDKCONFIG): defconfig
endif

endif 	# ifeq ("$(wildcard $(SDKCONFIG))","")
#-----------------------------------------------------------------

# macro for the commands to run kconfig tools conf or mconf.
# $1 is the name (& args) of the conf tool to run
define RunConf
	mkdir -p $(BUILD_DIR_BASE)/include/config
	cd $(BUILD_DIR_BASE); KCONFIG_AUTOHEADER=$(abspath $(BUILD_DIR_BASE)/include/autoconfig.h) \
	COMPONENT_KCONFIGS="$(COMPONENT_KCONFIGS)" KCONFIG_CONFIG=$(SDKCONFIG) \
	COMPONENT_KCONFIGS_PROJBUILD="$(COMPONENT_KCONFIGS_PROJBUILD)" \
	$(KCONFIG_TOOL_DIR)/$1 $(srctree)/Kconfig
endef

ifeq ("$(MAKE_RESTARTS)","")
# menuconfig, defconfig and "GENCONFIG" configuration generation only
# ever run on the first make pass, subsequent passes don't run these
# (make often wants to re-run them as the conf tool can regenerate the
# autoconfig input file as an output file, but this is not what the
# user wants - a single config pass is enough to produce all output
# files.)
#
# To prevent problems missing genconfig, ensure none of these targets
# depend on any prerequisite that may cause a make restart as part of
# the prerequisite's own recipe.

menuconfig: env_setup $(KCONFIG_TOOL_DIR)/mconf
	$(summary) $(YELLOW) MENUCONFIG $(NC)
ifdef BATCH_BUILD
	@echo "Can't run interactive configuration inside non-interactive build process."
	@echo ""
	@echo "Open a command line terminal and run 'make menuconfig' from there."
	@echo "See esp-idf documentation for more details."
	@exit 1
else
	$(call RunConf,mconf)
endif

# defconfig creates a default config, based on SDKCONFIG_DEFAULTS if present
defconfig: env_setup $(KCONFIG_TOOL_DIR)/conf toolchain
	$(summary) $(YELLOW) DEFCONFIG $(NC)
ifneq ("$(wildcard $(SDKCONFIG_DEFAULTS))","")
	cat $(SDKCONFIG_DEFAULTS) >> $(SDKCONFIG)  # append defaults to autoconfig, will override existing values
endif
	$(call RunConf,conf --olddefconfig)

%_defconfig: env_setup $(KCONFIG_TOOL_DIR)/conf toolchain
	@echo -e $(RED)"new target(%_defconfig): $(@)" $(NC)
	$(call RunConf,conf --defconfig=$(srctree)/configs/$@)

savedefconfig: env_setup $(KCONFIG_TOOL_DIR)/conf $(SDKCONFIG) $(COMPONENT_KCONFIGS)
	@echo -e $(RED)"new target(savedefconfig): $(@)" $(NC)
	$(call RunConf,conf --$@=defconfig)

# if neither defconfig or menuconfig are requested, use the GENCONFIG rule to
# ensure generated config files are up to date
$(SDKCONFIG_MAKEFILE) $(BUILD_DIR_BASE)/include/autoconfig.h: $(KCONFIG_TOOL_DIR)/conf $(SDKCONFIG) $(COMPONENT_KCONFIGS) $(COMPONENT_KCONFIGS_PROJBUILD) | $(call prereq_if_explicit,defconfig) $(call prereq_if_explicit,menuconfig) $(call prereq_if_explicit,savedefconfig) $(call prereq_if_explicit,%_defconfig)
	$(summary) $(YELLOW) GENCONFIG $(NC)
ifdef BATCH_BUILD  # can't prompt for new config values like on terminal
	$(call RunConf,conf --olddefconfig)
endif
	$(call RunConf,conf --silentoldconfig)
	touch $(SDKCONFIG_MAKEFILE) $(BUILD_DIR_BASE)/include/autoconfig.h  # ensure newer than autoconfig

else  # "$(MAKE_RESTARTS)" != ""
# on subsequent make passes, skip config generation entirely
defconfig:
menuconfig:
%_defconfig:
savedefconfig:
endif


config-clean:
	$(summary RM CONFIG)
	rm -rf $(BUILD_DIR_BASE)/include/config $(BUILD_DIR_BASE)/include/autoconfig.h

distclean:
	rm -fr $(BUILD_OUTPUT) $(KCONFIG_AUTO_FILES) $(srctree)/tools/toolchain/active
	$(MAKE) -C $(KCONFIG_TOOL_DIR) distclean
	$(MAKE) -C $(ASTYLE_TOOL_DIR) clean

.PHONY: $(PHONY)
