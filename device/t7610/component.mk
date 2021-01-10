#
# component Makefile
#

###
# Add linker arguments to the LDFLAGS for the app executable. Defaults to -l$(COMPONENT_NAME).
# If adding pre-compiled libraries to this directory, add them as absolute paths - ie $(COMPONENT_PATH)/libwhatever.a
COMPONENT_ADD_LDFLAGS +=

###
# Paths, relative to the component directory, which will be added to the include search path for all components in the project.
# Defaults to include if not overridden.
# If an include directory is only needed to compile this specific component, add it to COMPONENT_PRIV_INCLUDEDIRS instead.
COMPONENT_ADD_INCLUDEDIRS +=

###
# Directory paths, must be relative to the component directory,
# which will be added to the include search path for this component's source files only.
COMPONENT_PRIV_INCLUDEDIRS += ./ ./include ./bsp ./bsp/evb ./bsp/fpga

###
# Object files to compile. Default value is a .o file for each source file that is found in COMPONENT_SRCDIRS.
# Overriding this list allows you to exclude source files in COMPONENT_SRCDIRS that would otherwise be compiled.
COMPONENT_OBJS :=        \
    ./bsp/bsp.o          \
    ./bsp/evb/bsp_evb.o  \
    ./bsp/fpga/bsp_fpga.o \
    hal/hal.o            \
    hal/hal_uart.o       \
    hal/hal_scu.o        \
    hal/hal_wdt.o        \
    hal/hal_timer.o      \
    hal/hal_gpio.o       \
    hal/hal_fc.o         \
    hal/hal_ssp.o        \
    hal/hal_img.o        \
    startup-gcc_cm4.o    \
    system_cm4.o

ifeq ("$(CONFIG_ENABLE_ETH)", "y")
COMPONENT_OBJS += hal/hal_eth.o hal/phy.o hal/phy_lan8720.o
endif

ifeq ("$(CONFIG_CPU_ARM_CM4)","y")
COMPONENT_OBJS += lib/memcpy_armv7m.o lib/strcpy.o
else ifeq ("$(CONFIG_CPU_ARM_CM0)","y")
endif

###
# Directory paths, must be relative to the component directory, which will be searched for source files (*.cpp, *.c, *.S).
# Defaults to '.', ie the component directory itself.
# Override this to specify a different list of directories which contain source files.
COMPONENT_SRCDIRS += hal bsp bsp/evb bsp/fpga lib

###
# set CFLAGS
CFLAGS +=

###
# set CXXFLAGS
CXXFLAGS +=

###
# set LDLAGS
LDLAGS +=

