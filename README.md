#Spherical Wavetable Navigator

Firmware for the Spherical Wavetable Navigator, a Eurorack-format module from 4ms Company.

## Setting up your environment
You need to install the GCC ARM toolchain.
This project is known to compile with arm-none-eabi-gcc version 7.3.1.

You also may wish to install and IDE such as Eclipse. There are many resources online for setting up GCC ARM with Eclipse (as well as commerical software). This is totally optional. Instead of an IDE you can use your favorite text editor and a few commands on the command line which are given below.

### Mac OSX

For Mac OSX, follow these instructions to install brew and then the arm toolchain and st-link (taken from https://github.com/nitsky/homebrew-stm32):

	ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
	brew tap nitsky/stm32
	brew install arm-none-eabi-gcc
	brew install stlink

Installing stlink is only necessary if you plan to use the ST-LINK/V2 programmer. Another great alternative is the J-Link or J-Trace. An educational version is available for a reasonable price, which is great if your use qualifies as educational. Download the "J-Link Software and Documentation Pack" and the "Ozone - The J-Link Debugger" from [SEGGER downloads page](https://www.segger.com/downloads/jlink).

That's it! Continue below to **Clone the Projects**

### Linux

For linux, check your package manager to see if there is a package for arm-none-eabi-gcc or gcc-arm-embedded or gcc-arm-none-eabi. Or just download it here:

[Download GCC ARM toolchain from Launchpad](https://launchpad.net/gcc-arm-embedded/+download)

If you want If you want to use the ST-LINK or Discovery board to flash your DLD, install st-link from texane:

	sudo apt-get install git libusb-1.0.0-dev pkg-config autotools-dev
	cd (your work directory)
	git clone https://github.com/texane/stlink.git
	cd stlink
	./autogen.sh
	./configure
	make
	export PATH=$PATH:(your work directory)/stlink

The last command makes sure that the binary st-flash is in your PATH so that the Makefile can run it to program your module. 

On the other hand, if you want to use the J-Link or J-Trace to flash your SWN, download and install the latest version of [J-Link Software and Documentation Pack](https://www.segger.com/j-link-software.html). You also may want to download and install Ozone, the GUI debugger. It can be downloaded from the [same page](https://www.segger.com/downloads/jlink).

If you don't know what you want to use to program, see the **programmer** section below!

That's it! Continue below to **Clone the Projects**

### Windows
[Download GCC ARM toolchain from Launchpad](https://launchpad.net/gcc-arm-embedded/+download)

If you want to use the ST-LINK or Discovery board to program, download and Install [ST-UTIL](http://www.st.com/web/en/catalog/tools/PF258168).

If you want to use the J-Link or J-Trace to program, download and install the latest version of [J-Link Software and Documentation Pack](https://www.segger.com/j-link-software.html). You also may want to download and install Ozone, the GUI debugger. It can be downloaded from the [same page](https://www.segger.com/downloads/jlink).

If you don't know what you want to use to program, see the programmer section below!

Please contact me if you run into problems that you can't google your way out of.

## Clone the Projects

Make sure git is installed on your system. (OSX: type "brew install git" into the Terminal).

Create a work directory, and enter it:
 
	mkdir myhackdir && cd myhackdir

Clone the SWN repository:

	git clone https://github.com/4ms/SWN.git  
	cd SWN

## Compiling
Make your changes to the code in the SWN directory. When ready to compile, make the project like this:

	make
	
This creates an main.elf, main.bin, and main.hex file in the build/ directory. See the Programmer section below for how to get these files onto your SWN.

## Bootloader
The bootloader is in a folder called `bootloader`.
It's based on the stm-audio-bootloader from [pichenettes](https://github.com/pichenettes/eurorack). 

The bootloader is already installed on all factory-built SWN, so you don't need to mess with this project if you're just planning on modifying your SWN code. The bootloader allows you to replace the firmware by playing a compiled audio file into the Waveform In jack.

Doing `make` will build the bootloader and also use the main SWN app's main.hex file to create a combined hex file. This file is a complete flash dump of a factory SWN. After building the SWN first, and then the bootloader, the file will be in `SWN/bootloader/build/bootloader-app-combo/`. There is a hex and a bin version in that folder.

## Programmer

Once you can compile your firmware, you will need a way to flash it to your SWN. There are several options:

###Option 1) Audio Bootloader
The SWN has an audio bootloader built-in, so you can just compile your project using `make wav` and then play the wav file into the audio input of the SWN while in bootloader mode (see SWN User Manual for detailed procedure).
This works well and is the safest option. However it's very slow (up to 5 minutes per update). If you are going to be making a series of changes, this will be a very slow process!

When ready to build an audio file for the bootloader, make it like this:

	make wav

This requires python to run. It creates the file `main.wav` in the build/ directory. Play the file from a computer or device into the SWN by following the instructions in the User Manual on the [4ms SWN page](http://4mscompany.com/SWN). 


###Option 2) ST-UTIL or stlink

A faster way to flash firmware is to use ST-UTIL (windows) or stlink (osx or linux) to program the SWN over the 4-pin SWD header. With ST-UTIL or stlink and a programmer, you can update in a few seconds.

ST-UTIL only runs on Windows, and the stlink package from Texane runs on osx or linux.

####Discovery board programmer
Texane's stlink contains a gdb debugger that works with SWD programmers such as the [STM32 Discovery boards](http://www.mouser.com/search/ProductDetail.aspx?R=0virtualkey0virtualkeySTM32F407G-DISC1). The STM32F4 Discovery Board is low-cost (around US$25) and works great as a programmer and debugger. Using four wires (that you have to supply), you can connect to the first 4 pins on the SWN header to the SWN's 6-pin SWD header. Read the Discovery board's manual to learn about setting the jumpers to use it as an SWD programmer (rather than an evaluation board). ST makes many variations of the Discovery board, and to my knowledge they all contain an ST-LINK programmer which functions the same.

####ST-LINKv2 programmer
Another SWD programmer, which is slightly more robust than the Discovery board, is the [ST-LINK v2 from ST corporation](http://www.mouser.com/search/ProductDetail.aspx?R=0virtualkey0virtualkeyST-LINK-V2). This works similarly to the Discovery board. The ST-LINK v2 is encased in a plastic enclosure, while the Discovery board is open and could potentially be damaged if you're not careful. The ST-LINK v2 has a 20-pin JTAG header, so you have to read the pinout to know which four wires to connect to your SWN. More details on this are below...

If you have stlink installed (osx and linux only) you can flash the bin file by typing:

	st-flash write build/main.bin 0x08010000
	
	
This writes the main.bin file starting at the 0x08010000 sector.

In Windows, use ST-UTIL to write the main.bin file starting at 0x08010000.

####Oops, I overwrote the bootloader and now it won't boot...
The bootloader lives at 0x08000000, so if you write the main.bin/hex to that sector you will overwrite the bootloader. And then it won't boot. The best way is to rewrite the bootloader. Do this:

	st-flash write bootloader/build/bootloader-app-combo/SWN_combo.bin 0x08000000
	
Or if the combo is not working, you could try just the bootloader:

	st-flash write bootloader/build/bootloader/bootloader.bin 0x08000000

Or on Windows, just use ST-UTIL to flash the SWN_combo.hex file (which is in the bootloader/build/bootloader-app-combo/ folders)


###Option 3) SEGGER's J-Link or J-Trace

Another option is [SEGGER's J-link programmer](https://www.segger.com/jlink-debug-probes.html). There is an educational version which is very affordable and a good choice for a programmer if you meet the requirements for education use. There are various professional commercial versions with a number of useful features. The J-link uses SEGGER's propriety software, Ozone, which not only will flash a hex or bin file, but can be used as a powerful debugger if you use it to open the .elf file. There also are command-line commands for flashing. 
J-Link software runs on Mac, Linux, and Windows.

###Connecting the programmer to the SWN (options 2 and 3)

**Discovery Board**: The Discovery board has a 6-pin SWD header, and you can connect that pin-for-pin to the DLD's 6-pin SWD header. Only the first four pins are used, but it does no harm to connect pins 5 and 6. You can use a 0.1" spaced 4-pin Jumper Wire such as [this one from Sparkfun](https://www.sparkfun.com/products/10364).

**ST-LINK v2 or SEGGER J-Link**: Both the ST-LINK and the SEGGER J-link have a 20-pin JTAG connector. You need to connect 4 of these pins to the DLD's 6-pin SWD connector, using 4 wires: [here's a pack of 10](https://www.sparkfun.com/products/8430)
 
Look at these images:
 
  * JTAG: [20-pin pinout](http://www.jtagtest.com/pinouts/arm20)
  * SWD: [6-pin pinout](https://wiki.paparazziuav.org/wiki/File:Swd_header_discovery_board.png)
  * JTAG-to-SWD: [JTAG-to-SWD-pinout](https://4mscompany.com/JTAG-to-SWD-pinout.png). 

Then use your jumper wires to connect:
 
  * SWD pin 1 (VDD) -> JTAG pin 1 (VREF)
  * SWD pin 2 (SWCLK) -> JTAG pin 9 (TCK)
  * SWD pin 3 (GND) -> JTAG pin 4 (GND)
  * SWD pin 4 (SWDIO) -> JTAG pin 7 (TMS)

Here's more info in case you want it explained in another way:

  * [This image](https://www.alexwhittemore.com/wp-content/uploads/2014/08/schematic.jpg) draws this out. (Taken from [this post](https://www.alexwhittemore.com/st-linkv2-swd-jtag-adapter/))
  * [ST's manual](http://www.st.com/content/ccc/resource/technical/document/user_manual/70/fe/4a/3f/e7/e1/4f/7d/DM00039084.pdf/files/DM00039084.pdf/jcr:content/translations/en.DM00039084.pdf)
  * [How to make an adaptor](http://gnuarmeclipse.github.io/developer/j-link-stm32-boards/#the-st-6-pin-swd-connector)
  * Make your own adaptor from these gerber files: [JTAG-SWD-adaptor](https://github.com/4ms/pcb-JTAG-SWD-adaptor).


## License
The code (software) is licensed by the MIT license.

The hardware is licensed by the [CC BY-NC-SA license](https://creativecommons.org/licenses/by-nc-sa/4.0/) (Creative Commons, Attribution, NonCommercial, ShareAlike).

See LICENSE file.

I would like to see others build and modify the SWN and SWN-influenced works, in a non-commercial manner. My intent is not to limit the educational use nor to prevent people buying hardware components/PCBs collectively in a group. If you have any questions regarding the license or appropriate use, please do not hesitate to contact me! 

## Guidelines for derivative works

Do not include the text "4ms" or "4ms Company" or the graphic 4ms logo on any derivative works. This includes faceplates, enclosures, PCBs, and front-panels. It's OK (but not required) to include the text "Spherical Wavetable Navigator" or "SWN" if you wish.

