#
# component Makefile
#


ifeq ("$(CHIP)","STM32F072xB")
## STM32F072xB
C_DEFS += -DSTM32F072xB


C_SOURCES := \
    $(srctree)/Drivers/CMSIS/Device/ST/STM32F0xx/Source/gcc/startup_stm32f072xb.c  \
    $(srctree)/Drivers/CMSIS/Device/ST/STM32F0xx/Source/gcc/syscalls.c             \
    $(srctree)/Drivers/CMSIS/Device/ST/STM32F0xx/Source/system_stm32f0xx.c

C_SOURCES +=  \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.c                \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.c         \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.c		   \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_adc.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_adc_ex.c         \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_can.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cec.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_comp.c           \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_crc.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_crc_ex.c         \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dac.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dac_ex.c         \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dma.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_exti.c           \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash.c          \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash_ex.c       \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c           \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c_ex.c         \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2s.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_irda.c           \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_iwdg.c           \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pcd.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pcd_ex.c         \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.c         \
	$(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rtc.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rtc_ex.c         \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_smartcard.c      \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_smartcard_ex.c   \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_smbus.c          \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_spi.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_spi_ex.c         \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim_ex.c         \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tsc.c            \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_uart.c           \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_uart_ex.c        \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_usart.c          \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_usart_ex.c       \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_wwdg.c           \
    $(srctree)/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_usb.c             \
    $(srctree)/Drivers/BSP/STM32072B_EVAL/stm32072b_eval.c


C_INCLUDES += \
    -I$(srctree)/Drivers/CMSIS/Device/ST/STM32F0xx/Include \
    -I$(srctree)/Drivers/STM32F0xx_HAL_Driver/Inc \
    -I$(srctree)/Drivers/BSP/STM32072B_EVAL


else
## Unknown
$(error "Unknown Chip !!!")
endif



CFLAGS +=
CXXFLAGS +=
LDLAGS +=

