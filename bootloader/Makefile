# Makefile by Dan Green <danngreen1@gmail.com>
#

BINARYNAME = bootloader
MAINAPP_HEX = ../build/main.hex

APP_SOURCES = 	../src/drivers/codec_i2c.c \
				../src/drivers/codec_sai.c \
				../src/gpio_pins.c \
				../src/drivers/pca9685_driver.c \
				../src/hardware_controls.c \
				../src/flash.c \
				../src/hal_handlers.c \
				../src/led_map.c \
				../src/led_colors.c \
				../src/drivers/mono_led_driver.c \
				../src/drivers/rotary_driver.c \
				../src/drivers/switch_driver.c

BUILDDIR = build
BLBUILDDIR = $(BUILDDIR)/$(BINARYNAME)

COMBODIR = $(BUILDDIR)/bootloader-app-combo
COMBO = $(COMBODIR)/SWN_combo

STARTUP = startup_stm32f765xx.s
SYSTEM = system_stm32f7xx.c
LOADFILE = bootloader_STM32F765VGTx_FLASH.ld

DEVICE = ../stm32/device
CORE = ../stm32/core
PERIPH = ../stm32/periph

SOURCES += $(wildcard *.c)
SOURCES += $(wildcard *.cc)
# SOURCES += stm_audio_bootloader/qpsk/packet_decoder.cc
# SOURCES += stm_audio_bootloader/qpsk/demodulator.cc
SOURCES += stm_audio_bootloader/fsk/packet_decoder.cc
SOURCES += $(APP_SOURCES)
SOURCES += $(wildcard $(PERIPH)/src/*.c)
SOURCES += $(DEVICE)/src/$(STARTUP)
SOURCES += $(DEVICE)/src/$(SYSTEM)



OBJECTS = $(addprefix $(BLBUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

INCLUDES += -I$(DEVICE)/include \
			-I$(CORE)/include \
			-I$(PERIPH)/include \
			-I stm_audio_bootloader/fsk \
			-I stmlib \
			-I stmlib/utils \
			-I ../inc \
			-I ../inc/drivers


ELF = $(BLBUILDDIR)/$(BINARYNAME).elf
HEX = $(BLBUILDDIR)/$(BINARYNAME).hex
BIN = $(BLBUILDDIR)/$(BINARYNAME).bin

ARCH = arm-none-eabi
CC = $(ARCH)-gcc
CPP = $(ARCH)-g++
##Use -gcc instead of -ld
LD = $(ARCH)-gcc -Wl,-Map,build/main.map,--cref -Wl,--gc-sections
#LD = $(ARCH)-ld -v -Map main.map
AS = $(ARCH)-as
OBJCPY = $(ARCH)-objcopy
OBJDMP = $(ARCH)-objdump
CMDSIZE = $(ARCH)-size
GDB = $(ARCH)-gdb


CPU = -mcpu=cortex-m7
FPU = -mfpu=fpv5-sp-d16
FLOAT-ABI = -mfloat-abi=hard
MCU = $(CPU) -mthumb -mlittle-endian $(FPU) $(FLOAT-ABI)
F_CPU = 216000000L

ARCH_CFLAGS = -D'IS_BOOTLOADER=1' -DARM_MATH_CM7 -D'__FPU_PRESENT=1' -DUSE_HAL_DRIVER -DSTM32F765xx -DF_CPU=$(F_CPU)

OPTIMIZED_CFLAGS = -g2 -Os

CFLAGS  = $(OPTIMIZED_CFLAGS) 
CFLAGS += $(ARCH_CFLAGS) $(MCU)
CFLAGS += -I.  $(INCLUDES)
CFLAGS += -fdata-sections -ffunction-sections
# CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

C0FLAGS  = -O0 -g -Wall
C0FLAGS += $(ARCH_CFLAGS) $(MCU)
C0FLAGS += -I.  $(INCLUDES)

CPPFLAGS = -fno-exceptions

AFLAGS = $(MCU)

LDSCRIPT = $(LOADFILE)

LFLAGS  = $(MCU) -specs=nano.specs  -T $(LDSCRIPT) -lc -lm -lnosys


# Codec
# build/bootloader/../src/codec_i2c.o: CFLAGS = $(C0FLAGS)
# build/bootloader/../src/codec_sai.o: CFLAGS = $(C0FLAGS)
# build/bootloader/../stm32/periph/src/stm32f7xx_hal_i2c.o: CFLAGS = $(C0FLAGS)
# build/bootloader/../src/drivers/pca9685_driver.o: CFLAGS = $(C0FLAGS)
	
# Main
# $(BLBUILDDIR)/bootloader.o: CFLAGS = $(C0FLAGS)
# $(BLBUILDDIR)/bl_controls.o: CFLAGS = $(C0FLAGS)


combo: $(COMBO).bin 

$(COMBO).bin:  $(MAINAPP_HEX) $(HEX)
	mkdir -p $(COMBODIR)
	cat  $(MAINAPP_HEX) $(HEX) | \
	awk -f util/merge_hex.awk > $(COMBO).hex
	$(OBJCPY) -I ihex -O binary $(COMBO).hex $(COMBO).bin

all: Makefile $(BIN) $(HEX)

$(BIN): $(ELF)
	$(OBJCPY) -O binary $< $@
	$(OBJDMP) -x --syms $< > $(addsuffix .dmp, $(basename $<))
	$(CMDSIZE) $<
	ls -l $@ $<

$(HEX): $(ELF)
	$(OBJCPY) --output-target=ihex $< $@

$(ELF): $(OBJECTS) 
	$(LD) $(LFLAGS) -o $@ $(OBJECTS)


$(BLBUILDDIR)/%.o: %.c $(wildcard inc/*.h) $(wildcard inc/res/*.h)
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@


$(BLBUILDDIR)/%.o: %.cc $(wildcard inc/*.h) $(wildcard inc/res/*.h)
	mkdir -p $(dir $@)
	$(CPP) -c $(CFLAGS) $(CPPFLAGS) $< -o $@


$(BLBUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $< -o $@ > $(addprefix $(BLBUILDDIR)/, $(addsuffix .lst, $(basename $<)))


flash: $(BIN)
	st-flash write $(BIN) 0x8000000

clean:
	rm -rf $(BUILDDIR)
	