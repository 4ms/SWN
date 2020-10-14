# Makefile by Dan Green <danngreen1@gmail.com>
#

BINARYNAME 		= main

COMBO 			= build/combo
BOOTLOADER_DIR 	= bootloader
BOOTLOADER_HEX 	= bootloader/build/bootloader.hex

FIRMWARE_RELEASE_DIR = LOCAL/Firmwares
FIRMWARE_RELEASE_NAME = SWN_firmware


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
SOURCES  += $(wildcard src/*.cc)
SOURCES  += $(wildcard src/drivers/*.c)
SOURCES  += $(wildcard $(CORE)/src/*.c)
SOURCES  += $(wildcard $(CORE)/src/*.s)

OBJECTS   = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(sort $(basename $(SOURCES)))))

DEPS = $(OBJECTS:.o=.d)

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
CXX		= $(ARCH)-g++
LD 		= $(ARCH)-g++
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

ARCH_CFLAGS = 	-DARM_MATH_CM7 \
				-D'__FPU_PRESENT=1' \
				-DUSE_HAL_DRIVER \
				-DSTM32F765xx

OPTFLAG = -O3

CFLAGS = -g3 -Wall \
	$(ARCH_CFLAGS) $(MCU) \
	-I. $(INCLUDES) \
	-fno-common \
	-fdata-sections -ffunction-sections \
	# -specs=nano.specs \

DEPFLAGS = -MMD -MP -MF $(BUILDDIR)/$(basename $<).d

CXXFLAGS=$(CFLAGS) \
	-std=c++17 \
	-fno-rtti \
	-fno-exceptions \
	-ffreestanding \
	-Werror=return-type \
	-Wdouble-promotion \
	-Wno-register \

AFLAGS = $(MCU) 

LDSCRIPT = $(DEVICE)/$(LOADFILE)

##-lrdimon is required to use math.h, -lc is required because it defines __errno for librdimon
#LFLAGS  = $(MCU) -v --specs=nano.specs -T $(LDSCRIPT)  -lc -lrdimon

LFLAGS =  -Wl,-Map,build/main.map,--cref \
	-Wl,--gc-sections \
	-Wl,--start-group \
	$(MCU) \
	-T $(LDSCRIPT)
	# -specs=nano.specs -T $(LDSCRIPT) \

# build/src/hardware_tests.o: OPTFLAG = -O0

#-----------------------------------
# Uncomment to compile unoptimized:

# # Main:
# # -----
# build/src/main.o: OPTFLAG = -O0
#
# # LFOS
# # -------
# build/src/params_lfo.o: OPTFLAG = -O0
# build/src/params_lfo_clk.o: OPTFLAG = -O0
# build/src/params_lfo_period.o: OPTFLAG = -O0

# # Audio
# # ------
# build/src/oscillator.o: OPTFLAG = -O0
# build/src/audio_util.o: OPTFLAG = -O0
# build/src/wavetable_editing.o: OPTFLAG = -O0
# build/src/wavetable_saveload.o: OPTFLAG = -O0
# build/src/wavetable_recording.o: OPTFLAG = -O0
# build/src/wavetable_effects.o: OPTFLAG = -O0
# build/src/resample.o: OPTFLAG = -O0
# build/src/fft_filter.o: OPTFLAG = -O0


# # Parameters
# # ----------
# build/src/params_update.o: OPTFLAG = -O0
# build/src/params_wt_browse.o: OPTFLAG = -O0

# build/src/analog_conditioning.o: OPTFLAG = -O0
# build/src/UI_conditioning.o: OPTFLAG = -O0
# build/src/quantz_scales.o: OPTFLAG = -O0
# build/src/led_cont.o: OPTFLAG = -O0
# build/src/ui_modes.o: OPTFLAG = -O0


# Timers
# build/src/timekeeper.o: OPTFLAG = -O0

# # Special Modes
# # -------------
# build/src/calibration.o: OPTFLAG = -O0
# build/src/system_mode.o: OPTFLAG = -O0
# build/src/led_color_adjust.o: OPTFLAG = -O0
#
# build/src/preset_manager.o: OPTFLAG = -O0
# build/src/preset_manager_UI.o: OPTFLAG = -O0
# build/src/preset_manager_undo.o: OPTFLAG = -O0
#



# # Drivers:
# # --------
#
# ADC
# build/src/drivers/adc_builtin_driver.o: OPTFLAG = -O0
# build/src/drivers/ads8634_driver.o: OPTFLAG = -O0
# build/src/adc_interface.o: OPTFLAG = -O0
# build/src/analog_conditioning.o: OPTFLAG = -O0
#
# GPIO Setup
# build/src/gpio_pins.o: OPTFLAG = -O0
# build/src/hardware_controls.o: OPTFLAG = -O0
#
# GPIO Controls
# build/src/drivers/button_driver.o: OPTFLAG = -O0
# build/src/drivers/mono_led_driver.o: OPTFLAG = -O0
# build/src/drivers/rotary_driver.o: OPTFLAG = -O0
# build/src/drivers/switch_driver.o: OPTFLAG = -O0
#
# PWM LEDs
# build/src/drivers/pca9685_driver.o: OPTFLAG = -O0
# build/stm32/periph/src/stm32f7xx_hal_i2c.o: OPTFLAG = -O0
# build/src/drivers/leds_pwm.o: OPTFLAG = -O0
#
# PWM Timer outputs
# build/src/envout_pwm.o: OPTFLAG = -O0
#
# External Flash
# build/src/drivers/flash_S25FL127.o: OPTFLAG = -O0
# build/src/drivers/flashram_spidma.o: OPTFLAG = -O0
# build/src/sphere_flash_io.o: OPTFLAG = -O0
# build/src/wavetable_play_export.o: OPTFLAG = -O0

# Sel Bus
# build/src/drivers/uart_driver.o: OPTFLAG = -O0
# build/src/sel_bus.o: OPTFLAG = -O0
# build/stm32/periph/src/stm32f7xx_hal_uart.o: OPTFLAG = -O0
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
	@echo "Linking..."
	@$(LD) $(LFLAGS) -o $@ $(OBJECTS)

$(BUILDDIR)/%.o: %.c $(BUILDDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "Compiling $< at $(OPTFLAG)"
	@$(CC) -c $(DEPFLAGS) $(OPTFLAG) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.cpp $(BUILDDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "Compiling $< at $(OPTFLAG)"
	@$(CXX) -c $(DEPFLAGS) $(OPTFLAG) $(CXXFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.cc $(BUILDDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "Compiling $< at $(OPTFLAG)"
	@$(CXX) -c $(DEPFLAGS) $(OPTFLAG) $(CXXFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $< -o $@ > $(addprefix $(BUILDDIR)/, $(addsuffix .lst, $(basename $<)))

flash: $(BIN)
	st-flash write $(BIN) 0x08010000

clean:
	rm -rf $(BUILDDIR)

%.d: ;

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif

wav: fsk-wav

fsk-wav: $(BIN)
	export PYTHONPATH='.' && python2 stm_audio_bootloader/fsk/encoder.py \
		-s 44100 -b 16 -n 8 -z 4 -p 256 -g 16384 -k 1800 \
		$(BIN)

release: wav
	@read -p "Version (example: v2.0): " RELEASEVERSION && \
	mv "$(BUILDDIR)/$(BINARYNAME).wav" "$(FIRMWARE_RELEASE_DIR)/$(FIRMWARE_RELEASE_NAME)_$$RELEASEVERSION.wav" && \
	zip -j "$(FIRMWARE_RELEASE_DIR)/$(FIRMWARE_RELEASE_NAME)_$$RELEASEVERSION.zip" "$(FIRMWARE_RELEASE_DIR)/$(FIRMWARE_RELEASE_NAME)_$$RELEASEVERSION.wav"

