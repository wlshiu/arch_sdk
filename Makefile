# ------------------------------------------------
# Generic Makefile (based on gcc)
# Copyright (c) 2022, All Rights Reserved.
# @file    Makefile
# @author  Wei-Lun Hsu
# @version 1.0
# ------------------------------------------------

RED="\033[0;31m"
GREEN="\033[0;32m"
LIGHT_GREEN="\033[1;32m"
YELLOW="\033[0;33m"
LIGHT_YELLOW="\033[1;33m"
GREY="\033[0;37m"
BWHITE="\033[1;37m"
MAGENTA="\033[1;35m"
CYAN="\033[1;36m"
NC="\033[0m"


PHONY := gdb_server gdb cgdb flash all clean help tags TAGS cscope gtags
######################################
# target
######################################
ifeq ("$(TARGET)","")
TARGET = demo
endif


ifeq ("$(CHIP)","")
CHIP = STM32F072xB
endif

srctree=$(shell pwd)

export CHIP TARGET srctree

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og

#######################################
# paths
#######################################
# Build path
ifeq ("$(O)","")
OUT = build
else
OUT = $(O)
endif

# Project path
ifeq ("$(P)","")
PROJ = Templates
else
PROJ = $(P)
endif

$(info Active @$(PROJ))
#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
SZ = $(GCC_PATH)/$(PREFIX)size
GDB= $(GCC_PATH)/$(PREFIX)gdb
OBJCOPY = $(GCC_PATH)/$(PREFIX)objcopy
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
SZ = $(PREFIX)size
GDB= $(PREFIX)gdb
OBJCOPY = $(PREFIX)objcopy
endif
HEX = $(OBJCOPY) -O ihex
BIN = $(OBJCOPY) -O binary -S

OPENOCD=$(srctree)/Tools/openocd/bin/openocd

TARGET_GDB_SCRIPT:=$(srctree)/Tools/openocd/bin/cm0.gdb

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m0plus

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS =  \
	-DUSE_HAL_DRIVER

# AS includes
AS_INCLUDES =

# C includes
C_INCLUDES =  \
	-I$(srctree)/Drivers/CMSIS/Include \
	-I$(srctree)/Common


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -std=gnu99 -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -ggdb -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

export CFLAGS ASFLAGS C_INCLUDES AS_INCLUDES C_DEFS
#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = $(srctree)/Drivers/CMSIS/Device/ST/STM32F0xx/Source/gcc/STM32F072VBTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(OUT)/$(TARGET).map,--cref -Wl,--gc-sections

export LDFLAGS LDSCRIPT LIBS
######################################
# source
######################################
# C sources
include $(srctree)/Drivers/component.mk

C_SOURCES +=

include $(srctree)/Projects/component.mk

# ASM sources
# ASM_SOURCES = startup_stm32f072xb.S

V ?= $(VERBOSE)
ifeq ("$(V)","1")
    Q =
else
    Q = @
endif

# list of objects
OBJECTS = $(addprefix $(OUT)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(OUT)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

#######################################
# build the application
#######################################
# default action: build all
all: $(OUT)/$(TARGET).elf $(OUT)/$(TARGET).hex $(OUT)/$(TARGET).bin

$(OUT)/%.o: %.c Makefile | $(OUT)
	@echo -e "  CC $@"
	$(Q)$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(OUT)/$(notdir $(<:.c=.lst)) $< -o $@

$(OUT)/%.o: %.S Makefile | $(OUT)
	@echo -e "$(CYAN)  CC $@ $(NC)"
	$(Q)$(AS) -c $(CFLAGS) $< -o $@

$(OUT)/$(TARGET).elf: $(OBJECTS) Makefile
	$(Q)$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo -e $(GREEN)"\nSize $(OUT)/$(TARGET).elf" $(NC)
	$(Q)$(SZ) $@

$(OUT)/%.hex: $(OUT)/%.elf | $(OUT)
	@echo -e $(YELLOW) "Generate $(OUT)/$(TARGET).hex" $(NC)
	$(Q)$(HEX) $< $@

$(OUT)/%.bin: $(OUT)/%.elf | $(OUT)
	@echo -e $(YELLOW) "Generate $(OUT)/$(TARGET).bin" $(NC)
	$(Q)$(BIN) $< $@

$(OUT):
	mkdir $@

flash: $(OUT)/$(TARGET).elf
	@echo -e $(YELLOW) "Program to $(OUT)/$(TARGET).elf flash" $(NC)
	@$(srctree)/Tools/openocd/bin/z_prog_flash.sh $(OUT)/$(TARGET).elf

gdb_server: $(OUT)/$(TARGET).elf
	@${OPENOCD} -s $(srctree)/Tools/openocd/share/openocd/scripts -f interface/cmsis-dap.cfg -f target/stm32f0x.cfg

gdb: $(OUT)/$(TARGET).elf
	@echo -e $(YELLOW) "run GDB" $(NC)
	@echo -e "  load $(OUT)/$(TARGET).elf"
	$(Q)$(GDB) --cd=$(srctree) --directory=$(srctree) --command=$(TARGET_GDB_SCRIPT) $(OUT)/$(TARGET).elf

cgdb: $(OUT)/$(TARGET).elf
	@echo -e $(YELLOW) "run CGDB" $(NC)
	@echo -e "  load $(OUT)/$(TARGET).elf"
	$(Q)cgdb -d $(GDB) --cd=$(srctree) --directory=$(srctree) --command=$(TARGET_GDB_SCRIPT) $(OUT)/$(TARGET).elf

clean:
	@echo -e $(YELLOW) "Clean target" $(NC)
	rm -fR $(OUT)
	@rm -f GPATH GRTAGS GTAGS cscope.files
	@echo -e "done\n"

tags TAGS cscope gtags: $(OUT)
	@rm -f GPATH GRTAGS GTAGS
	@$(srctree)/Tools/scripts/tags.sh $@

help:
	@echo "----------------------------------------------------------------------"
	@echo "Useful make targets:"
	@echo ""
	@echo "  make all                   - Build all"
	@echo "  make clean                 - Remove most generated files"
	@echo ""
	@echo "  make flash                 - Program ELF file to flash of target"
	@echo ""
	@echo "  make gdb_server            - Start GDB server"
	@echo "  make gdb                   - Run GDB for development"
	@echo "  make cgdb                  - Run CGDB for development"
	@echo ""
	@echo "  make tags/TAGS             - Generate tags file for editors"
	@echo "  make cscope                - Generate cscope index"
	@echo "  make gtags                 - Generate GNU GLOBAL index"
	@echo ""
	@echo "  make V=0|1 [targets] 0 => quiet build (default), 1 => verbose build"
	@echo "  make O=dir [targets] Locate all output files in 'dir'"
	@echo "  make P=dir [targets] Select the active project by 'dir'"
	@echo "----------------------------------------------------------------------"

#######################################
# dependencies
#######################################
-include $(wildcard $(OUT)/*.d)

.PHONY: $(PHONY)
