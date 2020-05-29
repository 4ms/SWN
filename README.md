# Spherical Wavetable Navigator

Firmware for the Spherical Wavetable Navigator, a Eurorack-format module from 4ms Company.

## Setting up the gcc-arm toolchain
You need to install the GCC ARM toolchain.
This project is known to compile with arm-none-eabi-gcc version 8.2.1 (8-2019q4) and 7.3.1 (7-2018q2).

If you're new to embedded development and not comfortable on the command line, you may wish to install an IDE. ST has an official IDE based on Eclipse called [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html). Or you can roll-your-own Eclipse using [GCC ARM with Eclipse](https://gnu-mcu-eclipse.github.io/). There are also commerical software packages which can be easily found by searching.

Another option is to use a fully-featured text editor such as VSCode or Sublime Text. You can configure these to run commands to compile and flash/program your project onto your SWN, too. There are ARM debugging extensions available, too. On the other hand, if you're happiest using the command line and prefer to write code with vim, emacs, etc, you can also do everything without having to lift your hand from the keyboard except to twiddle knobs on the SWN.

### Using the Virtual Machine ###

4ms has a virtual machine that provides a consistant development environment for compiling. Check out this project and follow the instructions in its README: [4ms-dev-env](https://github.com/4ms/4ms-dev-env). The SWN repo comes pre-cloned when you start this virtual machine.

First, follow the instructions on [vagrant's site](https://www.vagrantup.com/intro/getting-started) to install vagrant and VirtualBox.

Then, building the SWN project is as simple as this:

```
git clone https://github.com/4ms/4ms-dev-env.git
cd 4ms-dev-env
vagrant up
vagrant ssh
cd SWN
make
```

At present, the 4ms-dev-env is only setup to compile, not to flash (program), or debug.
If you decide to use the virtual machine to compile, you'll still need to use your local system to program the compiled code onto SWN. Skip down to the **Programmer** section.

### Mac OSX using Brew ###

For Mac OSX, you can use brew to install the gcc-arm toolchain.
First, install brew if you haven't already:
```
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

Then, install the gcc-arm toolchain v7.3.1:

```
brew tap nitsky/stm32
brew install arm-none-eabi-gcc
```

Continue below to **Clone the Projects**

Alternatively, you can follow the _Linux orMac OSX_ instructions below to get v8.2.1.


### Linux or Mac OSX ###

For Linux or Mac OSX, download GNU ARM Embedded Toolchain version 8-2018-q4-major from [Launchpad](https://launchpad.net/gcc-arm-embedded/+download).

Direct links: [Mac OSX gcc-arm 8.2.1](https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2018q4/gcc-arm-none-eabi-8-2018-q4-major-mac.tar.bz2) ... [Linux gcc-arm 8.2.1](https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2018q4/gcc-arm-none-eabi-8-2018-q4-major-linux.tar.bz2).

Then unzip them: Double-clicking them on MacOS might work, or type this:

```
cd ~/Downloads ##whatever your download directory is
tar jxf gcc-arm-none-eabi-8-2018-q4-major-*.tar.bz2
```

Next, move the unzipped directory to somewhere sensible and add the `bin/` dir to your PATH:

```
mv gcc-arm-none-eabi-8-2018-q4-major $HOME/myprojectdir
echo "export PATH=$HOME/myprojectdir/gcc-arm-none-eabi-8-2018-q4-major/bin:\$PATH" >> $HOME/.bash_profile
source $HOME/.bash_profile

```

Of course, modify the above with the actual directory you want to put the toolchain, and your shell start-up script if you're using zsh or something else.

Continue below to **Clone the Projects**

### Windows
Download the GNU ARM Embedded Toolchain version 8-2018-q4-major from [Launchpad](https://launchpad.net/gcc-arm-embedded/+download). Download the Installer for your Windows verion (don't download the .zip file, download the .exe file).

Run the installer, and continue below.

## Clone the Project ##

If you are using the virtual machine, this will already be cloned for you, so skip ahead.

Make sure git is installed on your system. (OSX: type "brew install git" into the Terminal).

Create a work directory, and enter it:
 
	mkdir myhackdir && cd myhackdir

Clone the SWN repository:

	git clone https://github.com/4ms/SWN.git  
	cd SWN

## Compiling ##
Make your changes to the code in the SWN directory. When ready to compile, make the project like this:

	make
	
This creates an main.elf, main.bin, and main.hex file in the build/ directory. See the Programmer section below for how to get these files onto your SWN.


## Programmer (Hardware) ##

Once you compile your firmware, you need to flash it to your SWN. You will need a hardware device that connects to your computer's USB port. The other side of the programmer has a header that conencts to the SWN's 4-pin SWD header.

You have several options:

### Option 1) ST-LINKv2 programmer ###

The [ST-LINK v2 from ST corporation](http://www.mouser.com/search/ProductDetail.aspx?R=0virtualkey0virtualkeyST-LINK-V2) is the offical programmer from ST (who makes the chip that needs programming). The ST-LINK v2 has a 20-pin JTAG header, and you need to connect four of these to your SWN. More details on this are below.

The ST-LINKv2 programmer can be used with ST-LINK software for Mac, Linux, or Windows; or ST-UTIL software for Windows; or STM32CubeProgrammer software for any platform. 

### Option 2) Discovery board programmer ###

The [STM32 Discovery boards](http://www.mouser.com/search/ProductDetail.aspx?R=0virtualkey0virtualkeySTM32F407G-DISC1) are low-cost (around US$25) and work great as a programmer and debugger. The 'Disco' board is essentially an ST-LINKv2, plus some extra stuff. While the ST-LINK v2 is encased in a plastic enclosure, the Discovery board is open and could potentially be damaged if you're not careful. Read the Discovery board's manual to learn about setting the jumpers to use it as an SWD programmer (rather than an evaluation board). ST makes many variations of the Discovery board, and to my knowledge they all contain an ST-LINK programmer.

The Discovery board programmer is essentially an ST-LINKv2, so it can be used with all the same software that the ST-LINKv2 can (see above section).

### Option 3) SEGGER's J-Link or J-Trace ###

Another option is [SEGGER's J-link programmer](https://www.segger.com/jlink-debug-probes.html). There is an educational version which is very affordable and a good choice for a programmer if you meet the requirements for education use. There are various professional commercial versions with a number of useful features. The J-link uses SEGGER's propriety software, Ozone, which not only will flash a hex or bin file, but can be used as a powerful GUI debugger if you use it to open the .elf file. There also are command-line commands for flashing.
J-Link software runs on Mac, Linux, and Windows.

### Option 4) Audio Bootloader ###

The SWN has an audio bootloader built-in, so you can just compile your project using `make wav` and then play the wav file into the audio input of the SWN while in bootloader mode (see SWN User Manual for detailed procedure). No additional hardware or software is necessary.

This works well and is the safest option. However it's very slow (up to 5 minutes per update, or 11+ minutes if you include all new factory wavetables). If you are going to be making a series of changes, this will be a very slow process!

When ready to build an audio file for the bootloader, make it like this:

	make wav

This requires python to run. It creates the file `main.wav` in the `build/` directory. Play the file from a computer or device into the SWN by following the instructions in the User Manual on the [4ms SWN page](http://4mscompany.com/SWN). 


## Programmer (Software)

Depending on the hardware you chose, one or more of the following software options will work:

### ST-LINK ###

This software works with the ST-LINK v2 and the Discovery board to flash your SWN.

__MacOS:__

```
brew install stlink
```

__Linux:__

Use your package manager to install `stlink-tools`. See [stlink README](https://github.com/stlink-org/stlink) for direct links.

__Windows:__

Download the [latest binary (v1.3.0)](https://github.com/stlink-org/stlink/releases/download/v1.3.0/stlink-1.3.0-win64.zip). If that link doesn't work anymore, or you want to try a newer version, go to [stlink-org/stlink github page](https://github.com/stlink-org/stlink).

__Programming using st-link__

You can flash the compiled .bin file by typing:

	st-flash write build/main.bin 0x08010000
	
This writes the main.bin file starting at the 0x08010000 sector, which is the first sector of the application code.

__Debugging__

The gcc-arm toolchain includes a gdb debugger, but you'll need a server to use it. The st-link software includes st-util which will provide this. OpenOCD and J-link are two other options. Configuring debugging is beyond the scope of this README, but there are plenty of tutorials online. See the [stlink README](https://github.com/stlink-org/stlink) for a quick start.


### J-Link/Ozone ###

If you're using a J-Link or J-Trace hardware programmer to flash your SWN, download and install [Ozone](https://www.segger.com/downloads/jlink). Ozone is a GUI debugger which can program, too. For command-line options, consider [J-Link Software and Documentation Pack](https://www.segger.com/j-link-software.html), which contains the GDB server and other tools.

To program your SWN, open Ozone and then use the Project Wizard to create a new project. Select the STM32F765ZG for the chip. Use the default settings elsewhere. On the page that lets you select a file, select the main.elf file that you compiled. You can then save this project. To program, just click the green power/arrow symbol at the upper left.

You also can use the CLI tools to flash the hex/elf file, see J-Link documentation for details.

J-Link and Ozone is available for Mac, Windows, and Linux.

### STM32CubeProgrammer ###

The STM32CubeProgrammer is ST's official programaming software available for all platforms. [You can find it here](https://www.st.com/en/development-tools/stm32cubeprog.html)

Documentation is included with the download.


### ST-UTIL (Windows only) ###

ST-UTIL is a program from ST that runs on Windows only. You can find it on [ST.com](https://my.st.com/content/my_st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html)

It works with the ST-LINKv2 and Discovery boards.


## Connecting the programmer to the SWN ##

**Discovery Board**: The Discovery board has a 6-pin SWD header, and you can connect that pin-for-pin to the first 4 pins on the SWN's SWD header. You can use a 0.1" spaced 4-pin Jumper Wire such as [this one from Sparkfun](https://www.sparkfun.com/products/10364). Pay attention to Pin 1 (marked by "1" on the SWN board), or you risk damaging the ARM chip.

**ST-LINK v2 or SEGGER J-Link**: Both the ST-LINK and the SEGGER J-link have a 20-pin JTAG connector. You need to connect 4 of these pins to the SWN's 4-pin SWD connector, using 4 wires: [here's a pack of 10](https://www.sparkfun.com/products/8430). Pay attention to Pin 1 (marked by "1" on the SWN board), or you risk damaging the ARM chip.
 
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

## Bootloader Information ##

The bootloader is in a folder called `bootloader/`.
It's based on the stm-audio-bootloader from [pichenettes](https://github.com/pichenettes/eurorack). 

The bootloader is already installed on all factory-built SWN, so you don't need to mess with this project if you're just planning on modifying your SWN code. The bootloader allows you to replace the firmware by playing a compiled audio file into the Waveform In jack.

Doing `make` from within the `bootloader/` directory will build the bootloader and also use the main SWN app's `main.hex` file to create a combined hex file. This file is a complete flash dump of a factory SWN. After building the SWN first, and then the bootloader, the file will be in `SWN/bootloader/build/bootloader-app-combo/`. There is a hex and a bin version in that folder.

__Oops, I overwrote the bootloader and now it won't boot...__

The bootloader lives at 0x08000000, so if you overwrite that sector you will overwrite the bootloader. And then it won't boot. The best way is to re-flash the bootloader. Do this:

	st-flash write bootloader/build/bootloader-app-combo/SWN_combo.bin 0x08000000
	
Or if the combo is not working, you could try just the bootloader:

	st-flash write bootloader/build/bootloader/bootloader.bin 0x08000000

## License ##

The code (software) is licensed by the MIT license.

The hardware is licensed by the [CC BY-NC-SA license](https://creativecommons.org/licenses/by-nc-sa/4.0/) (Creative Commons, Attribution, NonCommercial, ShareAlike).

See LICENSE file.

I would like to see others build and modify the SWN and SWN-influenced works, in a non-commercial manner. My intent is not to limit the educational use nor to prevent people buying hardware components/PCBs collectively in a group. If you have any questions regarding the license or appropriate use, please do not hesitate to contact me! 

## Guidelines for derivative works ##

Do not include the text "4ms" or "4ms Company" or the graphic 4ms logo on any derivative works. This includes faceplates, enclosures, PCBs, and front-panels. It's OK (but not required) to include the text "Spherical Wavetable Navigator" or "SWN" if you wish.

