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
COMPONENT_PRIV_INCLUDEDIRS +=

###
# Object files to compile. Default value is a .o file for each source file that is found in COMPONENT_SRCDIRS.
# Overriding this list allows you to exclude source files in COMPONENT_SRCDIRS that would otherwise be compiled.
COMPONENT_OBJS := \
	./driver/misc.o                  \
	./driver/stm32f4xx_adc.o         \
	./driver/stm32f4xx_can.o         \
	./driver/stm32f4xx_crc.o         \
	./driver/stm32f4xx_cryp.o        \
	./driver/stm32f4xx_cryp_aes.o    \
	./driver/stm32f4xx_cryp_des.o    \
	./driver/stm32f4xx_cryp_tdes.o   \
	./driver/stm32f4xx_dac.o         \
	./driver/stm32f4xx_dbgmcu.o      \
	./driver/stm32f4xx_dcmi.o        \
	./driver/stm32f4xx_dma.o         \
	./driver/stm32f4xx_dma2d.o       \
	./driver/stm32f4xx_exti.o        \
	./driver/stm32f4xx_flash.o       \
	./driver/stm32f4xx_fmc.o         \
	./driver/stm32f4xx_gpio.o        \
	./driver/stm32f4xx_hash.o        \
	./driver/stm32f4xx_hash_md5.o    \
	./driver/stm32f4xx_hash_sha1.o   \
	./driver/stm32f4xx_i2c.o         \
	./driver/stm32f4xx_iwdg.o        \
	./driver/stm32f4xx_ltdc.o        \
	./driver/stm32f4xx_pwr.o         \
	./driver/stm32f4xx_rcc.o         \
	./driver/stm32f4xx_rng.o         \
	./driver/stm32f4xx_rtc.o         \
	./driver/stm32f4xx_sai.o         \
	./driver/stm32f4xx_sdio.o        \
	./driver/stm32f4xx_spi.o         \
	./driver/stm32f4xx_syscfg.o      \
	./driver/stm32f4xx_tim.o         \
	./driver/stm32f4xx_usart.o       \
	./driver/stm32f4xx_wwdg.o		 \
	./bsp/bsp.o		 			     \
	system_stm32f4xx.o


###
# Directory paths, must be relative to the component directory, which will be searched for source files (*.cpp, *.c, *.S).
# Defaults to '.', ie the component directory itself.
# Override this to specify a different list of directories which contain source files.
COMPONENT_SRCDIRS += driver bsp


ifeq ($(CONFIG_STM32F429I),y)
COMPONENT_OBJS += 	\
	./bsp/stm32f429_439/stm32f429i_discovery.o \
	./bsp/stm32f429_439/startup_stm32f429.o

COMPONENT_SRCDIRS += bsp/stm32f429_439

endif


###
# set CFLAGS
CFLAGS +=

###
# set CXXFLAGS
CXXFLAGS +=

###
# set LDLAGS
LDLAGS +=

