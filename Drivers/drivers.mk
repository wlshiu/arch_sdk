#
# component Makefile
#


ifeq ("$(CHIP)","STM32F072xB")
## STM32F072xB
include $(srctree)/Drivers/STM32F0xx_HAL_Driver/component.mk
else
## Unknown
$(error "Unknown Chip !!!")
endif



CFLAGS +=
CXXFLAGS +=
LDLAGS +=

