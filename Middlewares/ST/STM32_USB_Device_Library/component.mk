#
# component Makefile
#


C_SOURCES +=  \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/AUDIO/Src/usbd_audio.c          \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c              \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid.c  \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Src/usbd_dfu.c              \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/HID/Src/usbd_hid.c              \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc.c              \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_bot.c          \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_data.c         \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_scsi.c         \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c                  \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c                \
	$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c


C_INCLUDES += \
	-I$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Core/Inc 				\
	-I$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/AUDIO/Inc    	\
	-I$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc      	\
	-I$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc	\
	-I$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Inc      	\
	-I$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/HID/Inc      	\
	-I$(srctree)/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc



CFLAGS +=
CXXFLAGS +=
LDLAGS +=

