# Makefile by Dan Green <danngreen1@gmail.com>
#

BINARYNAME 		= main

COMBO 			= build/combo
BOOTLOADER_DIR 	= ../SWN-bootloader
BOOTLOADER_HEX 	= ../SWN-bootloader/bootloader.hex

STARTUP 		= startup_stm32f765xx.s
SYSTEM 			= system_stm32f7xx.c
LOADFILE 		= STM32F765ZGTx_FLASH.ld

DEVICE 			= stm32/device
CORE 			= stm32/core
PERIPH 			= stm32/periph

BUILDDIR 		= build

SOURCES  += $(wildcard $(PERIPH)/src/*.c)
SOURCES  += $(DEVICE)/src/$(STARTUP)
SOURCES  += $(DEVICE)/src/$(SYSTEM)
SOURCES  += $(wildcard src/*.c)
SOURCES  += $(wildcard src/drivers/*.c)
SOURCES  += $(wildcard $(CORE)/src/*.c)
SOURCES  += $(wildcard $(CORE)/src/*.s)


OBJECTS   = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

INCLUDES += -I$(DEVICE)/include \
			-I$(CORE)/include \
			-I$(PERIPH)/include \
			-I inc \
			-I inc/drivers \
			-I inc/tests

ELF 	= $(BUILDDIR)/$(BINARYNAME).elf
HEX 	= $(BUILDDIR)/$(BINARYNAME).hex
BIN 	= $(BUILDDIR)/$(BINARYNAME).bin

ARCH 	= arm-none-eabi
CC 		= $(ARCH)-gcc
LD 		= $(ARCH)-gcc -Wl,-Map,build/main.map
AS 		= $(ARCH)-as
OBJCPY 	= $(ARCH)-objcopy
OBJDMP 	= $(ARCH)-objdump
GDB 	= $(ARCH)-gdb
SZ 		= $(ARCH)-size

SZOPTS 	= -d

CPU = -mcpu=cortex-m7 
FPU = -mfpu=fpv5-d16
FLOAT-ABI = -mfloat-abi=hard 
MCU = $(CPU) -mthumb -mlittle-endian $(FPU) $(FLOAT-ABI) 

ARCH_CFLAGS = -DARM_MATH_CM7 -D'__FPU_PRESENT=1' -DUSE_HAL_DRIVER -DSTM32F765xx

OPTIMIZED_CFLAGS = -g2 -O3 -fno-common

CFLAGS  = $(OPTIMIZED_CFLAGS) -Wall
CFLAGS += $(ARCH_CFLAGS) $(MCU) 
CFLAGS += -I. $(INCLUDES) 
#CFLAGS += -fdata-sections -ffunction-sections
#CFLAGS += -fstack-usage -fstack-check

C0FLAGS  = -O0 -g -Wall
C0FLAGS += $(ARCH_CFLAGS) $(MCU)
C0FLAGS += -I.  $(INCLUDES)

AFLAGS = $(MCU) 
LDSCRIPT = $(DEVICE)/$(LOADFILE)
#-lrdimon is required to use math.h, -lc is required because it defines __errno for librdimon
LFLAGS  = $(MCU) -v --specs=nano.specs -T $(LDSCRIPT)  -lc -lrdimon

# build/src/hardware_tests.o: CFLAGS = $(C0FLAGS)

#-----------------------------------
# Uncomment to compile unoptimized:

# # Main:
# # -----
# build/src/main.o: CFLAGS = $(C0FLAGS)
#
# # LFOS
# # -------
# build/src/params_lfo.o: CFLAGS = $(C0FLAGS)
# build/src/params_lfo_clk.o: CFLAGS = $(C0FLAGS)
# build/src/params_lfo_period.o: CFLAGS = $(C0FLAGS)

# # Audio
# # ------
# build/src/oscillator.o: CFLAGS = $(C0FLAGS)
# build/src/audio_util.o: CFLAGS = $(C0FLAGS)
# build/src/wavetable_editing.o: CFLAGS = $(C0FLAGS)
# build/src/wavetable_saving.o: CFLAGS = $(C0FLAGS)
# build/src/wavetable_recording.o: CFLAGS = $(C0FLAGS)
# build/src/wavetable_effects.o: CFLAGS = $(C0FLAGS)
# build/src/resample.o: CFLAGS = $(C0FLAGS)
# build/src/fft_filter.o: CFLAGS = $(C0FLAGS)


# # Parameters
# # ----------
# build/src/params_update.o: CFLAGS = $(C0FLAGS)
# build/src/params_wt_browse.o: CFLAGS = $(C0FLAGS)

# build/src/analog_conditioning.o: CFLAGS = $(C0FLAGS)
# build/src/UI_conditioning.o: CFLAGS = $(C0FLAGS)
# build/src/quantz_scales.o: CFLAGS = $(C0FLAGS)
# build/src/led_cont.o: CFLAGS = $(C0FLAGS)
# build/src/ui_modes.o: CFLAGS = $(C0FLAGS)


# Timers
# build/src/timekeeper.o: CFLAGS = $(C0FLAGS)

# # Special Modes
# # -------------
# build/src/calibration.o: CFLAGS = $(C0FLAGS)
# build/src/system_mode.o: CFLAGS = $(C0FLAGS)
# build/src/led_color_adjust.o: CFLAGS = $(C0FLAGS)
#
# build/src/preset_manager.o: CFLAGS = $(C0FLAGS)
# build/src/preset_manager_UI.o: CFLAGS = $(C0FLAGS)
# build/src/preset_manager_undo.o: CFLAGS = $(C0FLAGS)
#



# # Drivers:
# # --------
#
# ADC
# build/src/drivers/adc_builtin_driver.o: CFLAGS = $(C0FLAGS)
# build/src/drivers/ads8634_driver.o: CFLAGS = $(C0FLAGS)
# build/src/adc_interface.o: CFLAGS = $(C0FLAGS)
# build/src/analog_conditioning.o: CFLAGS = $(C0FLAGS)
#
# GPIO Setup
# build/src/gpio_pins.o: CFLAGS = $(C0FLAGS)
# build/src/hardware_controls.o: CFLAGS = $(C0FLAGS)
#
# GPIO Controls
# build/src/drivers/button_driver.o: CFLAGS = $(C0FLAGS)
# build/src/drivers/mono_led_driver.o: CFLAGS = $(C0FLAGS)
# build/src/drivers/rotary_driver.o: CFLAGS = $(C0FLAGS)
# build/src/drivers/switch_driver.o: CFLAGS = $(C0FLAGS)
#
# PWM LEDs
build/src/drivers/pca9685_driver.o: CFLAGS = $(C0FLAGS)
# build/src/drivers/leds_pwm.o: CFLAGS = $(C0FLAGS)
#
# PWM Timer outputs
# build/src/envout_pwm.o: CFLAGS = $(C0FLAGS)
#
# External Flash
# build/src/drivers/flash_S25FL127.o: CFLAGS = $(C0FLAGS)
# build/src/sphere_flash_io.o: CFLAGS = $(C0FLAGS)


#-----------------------------------


all: Makefile $(BIN) $(HEX)

combo: $(COMBO).hex 
$(COMBO).hex:  $(BOOTLOADER_HEX) $(BIN) $(HEX)
	cat  $(HEX) $(BOOTLOADER_HEX) | \
	awk -f $(BOOTLOADER_DIR)/util/merge_hex.awk > $(COMBO).hex
	$(OBJCPY) -I ihex -O binary $(COMBO).hex $(COMBO).bin


$(BIN): $(ELF)
	$(OBJCPY) -O binary $< $@
	$(OBJDMP) -x --syms $< > $(addsuffix .dmp, $(basename $<))
	ls -l $@ $<


$(HEX): $(ELF)
	$(OBJCPY) --output-target=ihex $< $@
	$(SZ) $(SZOPTS) $(ELF)

$(ELF): $(OBJECTS) 
	$(LD) $(LFLAGS) -o $@ $(OBJECTS)


$(BUILDDIR)/%.o: %.c $(wildcard inc/*.h) $(wildcard inc/drivers/*.h) $(wildcard inc/tests/*.h)
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@


# build/stm32/core/src/arm_bitreversal2.o: AS = $(ARCH)-gcc -x assembler-with-cpp

$(BUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $< -o $@ > $(addprefix $(BUILDDIR)/, $(addsuffix .lst, $(basename $<)))


flash: $(BIN)
	st-flash write $(BIN) 0x08010000

clean:
	rm -rf build
	
wav: fsk-wav

qpsk-wav: $(BIN)
	python stm_audio_bootloader/qpsk/encoder.py \
		-t stm32f4 -s 48000 -b 12000 -c 6000 -p 256 \
		$(BIN)

fsk-wav: $(BIN)
	python stm_audio_bootloader/fsk/encoder.py \
		-s 44100 -b 16 -n 8 -z 4 -p 256 -g 16384 -k 1800 \
		$(BIN)
