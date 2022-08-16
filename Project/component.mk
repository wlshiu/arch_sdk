#
# component Makefile
#

C_SOURCES += \
	$(srctree)/$(PROJ)/Templates/Src/main.c              \
	$(srctree)/$(PROJ)/Templates/Src/stm32f0xx_hal_msp.c  \
	$(srctree)/$(PROJ)/Templates/Src/stm32f0xx_it.c

C_INCLUDES += -I$(srctree)/$(PROJ)/Templates/Inc


LIBS +=

CFLAGS +=
LDLAGS +=

