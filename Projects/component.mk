#
# component Makefile
#

C_SOURCES += \
	$(srctree)/Projects/$(PROJ)/Src/main.c               \
	$(srctree)/Projects/$(PROJ)/Src/stm32f0xx_hal_msp.c  \
	$(srctree)/Projects/$(PROJ)/Src/stm32f0xx_it.c

C_INCLUDES += -I$(srctree)/Projects/$(PROJ)/Inc


LIBS +=

CFLAGS +=
LDLAGS +=

