#
# component Makefile
#

include $(srctree)/Middlewares/ST/STM32_USB_Device_Library/component.mk


C_SOURCES += \
	$(srctree)/Projects/$(PROJ)/Src/main.c                  \
	$(srctree)/Projects/$(PROJ)/Src/stm32f0xx_hal_msp.c     \
	$(srctree)/Projects/$(PROJ)/Src/stm32f0xx_it.c          \
	$(srctree)/Projects/$(PROJ)/Src/usbd_cdc_interface.c    \
	$(srctree)/Projects/$(PROJ)/Src/usbd_conf.c             \
	$(srctree)/Projects/$(PROJ)/Src/usbd_desc.c

C_INCLUDES += -I$(srctree)/Projects/$(PROJ)/Inc


LIBS +=

CFLAGS +=
LDLAGS +=

