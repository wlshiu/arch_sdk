# Makefile support for the menuconfig system

#Find all Kconfig files for all components
MODULE_KCONFIGS := $(foreach component,$(COMPONENT_PATHS),$(wildcard $(component)/Kconfig))
MODULE_KCONFIGS_PROJBUILD := $(foreach component,$(COMPONENT_PATHS),$(wildcard $(component)/Kconfig.projbuild))

# For doing make menuconfig etc
KCONFIG_TOOL_DIR=$(srctree)/tools/scripts/kconfig

# clear MAKEFLAGS as the menuconfig makefile uses implicit compile rules
$(KCONFIG_TOOL_DIR)/mconf $(KCONFIG_TOOL_DIR)/conf:
	MAKEFLAGS="" \
	CC=$(HOSTCC) LD=$(HOSTLD) \
	$(MAKE) -C $(KCONFIG_TOOL_DIR)
	# search apps folder to generate Kconfig of apps

menuconfig: $(KCONFIG_TOOL_DIR)/mconf $(srctree)/Kconfig $(BUILD_DIR_BASE)
	$(summary) MENUCONFIG...
	$(Q) KCONFIG_AUTOHEADER=$(BUILD_OUTPUT)/include/autoconf.h \
	KCONFIG_CONFIG=$(srctree)/.config \
	MODULE_KCONFIGS="$(MODULE_KCONFIGS)" \
	MODULE_KCONFIGS_PROJBUILD="$(MODULE_KCONFIGS_PROJBUILD)" \
	$(KCONFIG_TOOL_DIR)/mconf $(srctree)/Kconfig

ifeq ("$(wildcard $(PROJECT_PATH)/.config)","")
# No .config found. Need to run menuconfig to make this if we need it.
$(PROJECT_PATH)/.config: menuconfig
endif

defconfig: $(KCONFIG_TOOL_DIR)/mconf $(srctree)/Kconfig $(BUILD_DIR_BASE)
	$(summary) DEFCONFIG...
	$(Q) mkdir -p $(BUILD_OUTPUT)/include/config
	$(Q) KCONFIG_AUTOHEADER=$(BUILD_OUTPUT)/include/autoconf.h \
	KCONFIG_CONFIG=$(PROJECT_PATH)/.config \
	MODULE_KCONFIGS="$(MODULE_KCONFIGS)" \
	MODULE_KCONFIGS_PROJBUILD="$(MODULE_KCONFIGS_PROJBUILD)" \
	$(KCONFIG_TOOL_DIR)/conf --olddefconfig $(srctree)/Kconfig

# Work out of whether we have to build the Kconfig makefile
# (auto.conf), or if we're in a situation where we don't need it
NON_CONFIG_TARGETS := clean %-clean get_variable help menuconfig defconfig
AUTO_CONF_REGEN_TARGET := $(BUILD_OUTPUT)/include/config/auto.conf

# disable AUTO_CONF_REGEN_TARGET if all targets are non-config targets
# (and not building default target)
ifneq ("$(MAKECMDGOALS)","")
ifeq ($(filter $(NON_CONFIG_TARGETS), $(MAKECMDGOALS)),$(MAKECMDGOALS))
AUTO_CONF_REGEN_TARGET :=
# dummy target
$(BUILD_OUTPUT)/include/config/auto.conf:
endif
endif

$(AUTO_CONF_REGEN_TARGET) $(BUILD_OUTPUT)/include/autoconf.h: $(PROJECT_PATH)/.config $(KCONFIG_TOOL_DIR)/conf $(MODULE_KCONFIGS) $(MODULE_KCONFIGS_PROJBUILD)
	$(summary) GENCONFIG...
	$(Q) mkdir -p $(BUILD_OUTPUT)/include/config
	$(Q) cd build; KCONFIG_AUTOHEADER="$(BUILD_OUTPUT)/include/autoconf.h" \
	KCONFIG_CONFIG=$(PROJECT_PATH)/.config \
	MODULE_KCONFIGS="$(MODULE_KCONFIGS)" \
	MODULE_KCONFIGS_PROJBUILD="$(MODULE_KCONFIGS_PROJBUILD)" \
	$(KCONFIG_TOOL_DIR)/conf --silentoldconfig $(srctree)/Kconfig
	$(Q) touch $(AUTO_CONF_REGEN_TARGET) $(BUILD_OUTPUT)/include/autoconf.h
# touch to ensure both output files are newer - as 'conf' can also update .config (a dependency). Without this,
# sometimes you can get an infinite make loop on Windows where .config always gets regenerated newer
# than the target(!)

clean: config-clean
.PHONY: config-clean
config-clean:
	$(summary RM CONFIG)
	$(MAKE) -C $(KCONFIG_TOOL_DIR) clean
	$(Q) rm -rf $(BUILD_OUTPUT)/include/config $(BUILD_OUTPUT)/include/autoconf.h
