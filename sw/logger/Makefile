TARGET        = HighSpeedLogger

EXECUTABLE    = $(TARGET).elf
BIN_IMAGE     = $(TARGET).bin

CROSS        ?= arm-none-eabi-
# CROSS        ?= ~/sat/bin/arm-none-eabi-
#CROSS       ?= /opt/paparazzi/arm-multilib/bin/arm-none-eabi-

CC            = $(CROSS)gcc
GDB           = $(CROSS)gdb
OBJCOPY       = $(CROSS)objcopy
RANLIB        = $(CROSS)ranlib
AR            = $(CROSS)ar
GDB           = $(CROSS)gdb

CFLAGS        = -g -O0 -mlittle-endian -mthumb -DHSE_VALUE=12000000
CFLAGS       += -mcpu=cortex-m4	
CFLAGS       += -ffreestanding -nostdlib

# to run from FLASH
CFLAGS       += -Wl,-T,stm32_flash.ld

STMLIB_PATH   = ../libs/STM32F4xx_DSP_StdPeriph_Lib_V1.1.0/
STDPH_PATH    = ${STMLIB_PATH}/Libraries/STM32F4xx_StdPeriph_Driver/
DEVSUP_PATH   = ${STMLIB_PATH}/Libraries/CMSIS/Device/ST/STM32F4xx/Include/
CORESUP_PATH  = ${STMLIB_PATH}/Libraries/CMSIS/Include/

CFLAGS+=-I./
CFLAGS+=-I${STDPH_PATH}/inc
CFLAGS+=-I${DEVSUP_PATH}
CFLAGS+=-I${CORESUP_PATH} 
CFLAGS+=-I../libs/STM32_USB_Device_Library/Core/inc
CFLAGS+=-I../libs/STM32_USB_Device_Library/Class/cdc/inc
CFLAGS+=-I../libs/STM32_USB_OTG_Driver/inc
CFLAGS+=-DUSE_USB_OTG_FS=1
CFLAGS+=-DSTM32F40XX -DUSE_STDPERIPH_DRIVER=1

# Standard Peripheral Library
STDP_LIB = libSTM32F4xx_StdPeriph_Driver.a

STDP_SRCS = \
${STDPH_PATH}/src/misc.c \
${STDPH_PATH}/src/stm32f4xx_adc.c \
${STDPH_PATH}/src/stm32f4xx_can.c \
${STDPH_PATH}/src/stm32f4xx_crc.c \
${STDPH_PATH}/src/stm32f4xx_cryp_aes.c \
${STDPH_PATH}/src/stm32f4xx_cryp_des.c \
${STDPH_PATH}/src/stm32f4xx_cryp_tdes.c \
${STDPH_PATH}/src/stm32f4xx_cryp_des.c \
${STDPH_PATH}/src/stm32f4xx_cryp.c \
${STDPH_PATH}/src/stm32f4xx_dac.c \
${STDPH_PATH}/src/stm32f4xx_dbgmcu.c \
${STDPH_PATH}/src/stm32f4xx_dcmi.c \
${STDPH_PATH}/src/stm32f4xx_dma.c \
${STDPH_PATH}/src/stm32f4xx_exti.c \
${STDPH_PATH}/src/stm32f4xx_flash.c \
${STDPH_PATH}/src/stm32f4xx_fsmc.c \
${STDPH_PATH}/src/stm32f4xx_gpio.c \
${STDPH_PATH}/src/stm32f4xx_hash_md5.c \
${STDPH_PATH}/src/stm32f4xx_hash_sha1.c \
${STDPH_PATH}/src/stm32f4xx_hash.c \
${STDPH_PATH}/src/stm32f4xx_i2c.c \
${STDPH_PATH}/src/stm32f4xx_iwdg.c \
${STDPH_PATH}/src/stm32f4xx_pwr.c \
${STDPH_PATH}/src/stm32f4xx_rcc.c \
${STDPH_PATH}/src/stm32f4xx_rng.c \
${STDPH_PATH}/src/stm32f4xx_rtc.c \
${STDPH_PATH}/src/stm32f4xx_sdio.c \
${STDPH_PATH}/src/stm32f4xx_spi.c \
${STDPH_PATH}/src/stm32f4xx_syscfg.c \
${STDPH_PATH}/src/stm32f4xx_tim.c \
${STDPH_PATH}/src/stm32f4xx_usart.c \
${STDPH_PATH}/src/stm32f4xx_wwdg.c 

STDP_OBJS = $(STDP_SRCS:.c=.o)

USB_SRCS = \
../libs/STM32_USB_Device_Library/Class/cdc/src/usbd_cdc_core.c \
../libs/STM32_USB_Device_Library/Core/src/usbd_core.c  \
../libs/STM32_USB_Device_Library/Core/src/usbd_ioreq.c \
../libs/STM32_USB_Device_Library/Core/src/usbd_req.c \
../libs/STM32_USB_OTG_Driver/src/usb_dcd.c      \
../libs/STM32_USB_OTG_Driver/src/usb_dcd_int.c  

all: $(BIN_IMAGE)

$(BIN_IMAGE): $(EXECUTABLE)
	$(OBJCOPY) -O binary $^ $@

MAIN_SRCS = \
usbd_cdc_vcp.c \
main.c \
buffer_helpers.c \
endUserConfig.c \
logsystem.c \
dataInput.c \
system_stm32f4xx.c \
stm32f4xx_it.c \
sdio_sd.c \
leds.c \
buffer_helpers.c \
usb_bsp.c \
usbd_desc.c \
usbd_usr.c \
usb_core.c 

MAIN_OBJS = $(MAIN_SRCS:.c=.o)

$(EXECUTABLE): 	\
		startup_stm32f4xx.s \
		$(MAIN_OBJS) \
		${USB_SRCS} \
		${STDP_LIB}

	$(CC) $(CFLAGS) $^ -o $@  -L./ -lSTM32F4xx_StdPeriph_Driver -lgcc -lc

clean:
	rm -rf $(EXECUTABLE)
	rm -rf $(BIN_IMAGE)
	rm -rf *.lst *.o

mrproper:
	make clean -C ./
	make stdplib_clean -C ./

com:
	minicom -D /dev/ttyACM2 -o -w -t ansi

ifeq ($(BMP_PORT),)
upload:
#       /home/dirk/nonSynced/ARMnative/stlink.git/st-flash write ${BIN_IMAGE} 0x08000000
	st-flash write ${BIN_IMAGE} 0x08000000
else
upload:
	@echo "Using Black Magic Probe"
	@echo "Using GDB = $(GDB)"
	$(GDB) --batch \
                -ex 'target extended-remote $(BMP_PORT)' \
                -x black_magic_probe_flash.scr ${EXECUTABLE} 
endif

stdplib: $(STDP_LIB)

$(STDP_LIB): $(STDP_OBJS)
	$(AR) -r $(STDP_LIB) $(STDP_OBJS)
	$(RANLIB) $(STDP_LIB)

stdplib_clean:
	-rm -f $(STDP_OBJS)
	-rm -f $(STDP_LIB)

lst_rawi2s:
	$(CC) -c -g -Wa,-a,-ad $(CFLAGS) rawI2sDma.c > rawI2sDma.lst

#start:
#	export PATH=~/sat/bin/:$PATH

.PHONY: all clean 
