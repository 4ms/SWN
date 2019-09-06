#SWN Hardware Test
March 1, 2019

###Preparation

####Enter Hardware Test mode:

   1. If module has never passed the hardware test, it will automatically enter this mode until it passes.
   2. If it’s already passed, then hold down Button D + Transpose + LFO Shape while powering on.


####Prepare Scope:

* ⅛” tester cable
* “Normal” Time Mode (Horiz button)
* Horizontal: 2ms/div
* Channel’s Vertical: 5V/div
* Auto Trigger on the tester cable’s channel
* Trigger threshold at 50%
* Measurement enabled for Frequency and Amplitude


Prepare speakers/headphone to listen to what’s being scoped.

###Slider LED Test
1. Press center rotary to test each slider LED, one a at time. After the sixth press, all six sliders turn on at the same time.
2. Verify brightness/color.
3. Push the sliders to center now (you’ll see why in the Controls test).
4. Press rotary again to go to next test.

Note: It’s OK if the rest of the RGB lights are partially on/off: they don’t get tested until the next step.

_Fail_: "Slider LED" if a slider LED does not turn on or is always on, or two slider LEDs turn on at the same time. 

_Fail_: "Slider LED discolored" if a slider LED is substantially different hue than other slider LEDs.


###RGB LED Test

####Part 1: PCA I2C check
SWN automatically performs PCA chip communication test on I2C bus.

_Fail_: “PCA I2C Timeout”, if sliders E and F blink and sliders A/B/C/D show binary code for chip# that failed. Record fail and press rotary to continue. Otherwise, it automatically goes to Part 2.


####Part 2: All LEDs check
All light pipes, buttons, and rotary LEDs will turn white (Clock In LED will be blue and LFO buttons will be pink-ish).

_Fail_: “LED out”, if an LED is not white, or excessively dim or bright.

If it detects an error, it will automatically go to Part 3. Otherwise, you must make a choice here: 

  * If the lights definitely look good, the press the rotary to skip to Controls Test. 
  * If you want to double-check an LED individually, hold the rotary for 3 seconds unti the sliders flicker. Then release to go to Part 3 (individual LED test). 


####Part 3: Individual LED test (optional): 
Spin center rotary or Preset rotary to select one LED at a time to turn on. Each Red/Green/Blue is turned on separately.

_Fail_: “LED out/LEDs shorted”: If there’s any point in time when there are no LEDs on, or there are more than one LEDs on (including any color other than pure red, pure green, or pure blue). Note that you can use either the Preset rotary or the center rotary: you may not be able to conduct this test if there are issues with both of these rotaries. 
Press center rotary to continue to next test.


###Controls Test (rotaries, sliders, buttons, switches)

Staring state:

* Channel buttons are white (might be cyan or magenta if a slider is not centered)
* LFO buttons are blue
* LFO LEDs and LFO CV LED are white
* LED encoders are white


The goal is to turn all the lights off by pushing every button, turning every knob, moving every slider, and flipping the switch.

1. Slide each slider up and down fully:
   * Slider light should turn on when slider is >50% up (40-60% is ok)
   * Slider light should turn off when slider is <50% up (40-60% is ok)
   * Button above slider turns its red off when slider hits 100% (white=>cyan)
   * Button above slider turns its green off when slider hits 0% (cyan=>blue)


1. All 8 buttons should be blue now. Press each button to turn the blue color off.
2. For each rotary:
   - Each Rotary has an LED associated with it:
      * Octave: LFO LED B
      * LFO Speed: LFO LED A
      * Preset: LFO LED C
      * Depth: Depth (itself)
      * Center: LFO LED D
      * Latitude: Latitude (itself)
      * Longitude: Longitude (itself)
      * LFO Shape: LFO LED F
      * Transpose: LFO LED E
   - Turn it up and the light will turn cyan
   - Turn it down and the light will turn blue
   - Push its button and the light will turn off
   - While turning, watch light ring to make sure it’s not skipping steps
3. Press Fine to turn off the red color of the LFO CV LED (white=>cyan)
4. Flip V/oct switch up to turn off the green color of the LFO CV LED (cyan=>blue)
5. Flip V/oct switch down to turn off the blue color of the LFO CV LED (blue=>off)
6. Verify all lights are off.

  - _Fail_ “Rotary skipping” if light ring animation skips around when turning rotary.
  - _Fail_ “Rotary not responding to turning” if a rotary’s light is staying on any color except pure blue.
  - _Fail_ “Button not responding” if a button or rotary’s light is staying on pure blue.
  - _Fail_ “Fine button not responding” if LFO CV LED won’t turn off its red color
  - _Fail_ “Voct Switch stuck high/low” if LFO CV LED won’t turn off its blue/green color
  - _Fail_ “Slider not reaching max/min” if channel button won’t turn off its red or green color
  - _Fail_ “Slider voltage issue” if slider doesn’t turn on/off at about 50%
  - _Fail_ “Button/Rotary/Switch/Slider shorted” if multiple things happen when you adjust one control.


Hold down center rotary 3 seconds to proceed to next test. Sliders will flicker when it’s ok to let go


###Codec Communication Test
Automatic test (no user input required unless there’s a problem):

1. Waveform In LED turns blue while it tests the SAI initialization.
   - _Fail_ “Codec SAI error”, if slider F blinks quickly and Waveform LED is blue.
2. Waveform In LED turns red while it tests the I2C initialization.
3. Waveform In LED turns green while it tests the I2C communication.
  - _Fail_ “Codec I2C init error” if slider E blinks slowly and Waveform LED is green.
  - _Fail_ “Codec I2C register error”, if slider E blinks quickly and Waveform LED is green.

4. If there's an error, press rotary to continue, otherwise it automatically goes to next test.

###Outputs Test: Audio and LFOs

LFO LEDs will be Red/Green/Blue/Yellow/Cyan/Magenta

1. Patch scope and speakers into each Env jack and verify (approximate semi-tones):
   * Env A outputs a 248Hz left-leaning triangle, 7.2V amplitude.
   * Env B outputs a 257Hz left-leaning triangle, 7.2V amplitude.
   * Env C outputs a 287Hz left-leaning triangle, 7.2V amplitude.
   * Env D outputs a 313Hz left-leaning triangle, 6.8V amplitude.
   * Env E outputs a 342Hz left-leaning triangle, 6.8V amplitude.
   * Env F outputs a 360Hz left-leaning triangle, 6.6V amplitude.


2. Patch scope and speakers to Right OUT jack output. Scope should be measuring Freq and Amplitude.
3. Should see ~100Hz triangle with faster rise than fall (left-leaning). Amplitude = about 10V
  - _Fail_ “Audio Out amplitude too high/low” if amplitude is over/under 9.5V or 10.5V (or clipping)
  - _Fail_ “Audio OUT frequency” if wave is not between 99Hz and 101Hz,
  - _Fail_ “Audio OUT noisy/distorted” if you see noise or its distorted.
  - _Fail_ “Audio OUT DC offset” if wave is not centered around 0V.
  - _Fail_ “Audio OUT inverted” if wave is right-leaning


4. Patch a dummy cable into Left OUT jack. Immediately audio should go silent. This is because the normalization between Left and Right OUT jacks is broken by pluggin into the Left jack.
  - _Fail_ “Audio jack normalization issue” if right out jack is not silent when a cable is patched into left out (make sure nothing is patched into Waveform In, or it’s not really a failure).


5. Patch other end of the cable into Waveform In jack.  Should see inverted waveform (right-leaning), with amplitude = 19V
Do troubleshooting below if clipping, or amplitude < 18V, or no signal at all.


####Audio Test troubleshooting steps: 
Background Info: The Audio Test code copies the signal on the codec’s input to the codec’s right output (which goes to the Right OUT jack). The code also generates a 100Hz left-leaning triangle and outputs that on the codec’s left output (which goes to the Left OUT jack). The Left OUT jack’s switch lug is connected to the Right OUT jack’s signal lug, so if the Left OUT jack is unpatched, the Right OUT jack will play a 50/50 mix of the triangle wave and whatever’s on the Waveform In jack. This 50/50 mix means the amplitude of the left-leaning triangle will be halved. Also, the Waveform In’s analog circuitry has a low-pass filter whic makes -3dB at 3kHz, -6dB (50% of amplitude) at 6kHz, etc.


#####Additional tests if a fail was discovered:
1. Patch scope directly to left OUT jack. You should see the 100Hz triangle (with faster rise than fall) at 19.7V amplitude and no clipping. If so, then you know the codec is working and the left output channel is working. 
  - _Fail_ “Audio Left OUT no output” if no 100Hz signal on Left OUT jack.
  - _Fail_ “Audio Left OUT distorted/amplitude/clipped/offset” if signal is present but has an issue.


2. Patch a 100Hz sine wave into Waveform In jack. Patch a dummy cable into Left OUT jack. You should see the sine wave on Right OUT jack with a little more amplitude (input a 10V, 100Hz sine, it’ll output 10.2V amplitude sine). If so, then you know the input stage and right output stages are working. Since there is a LPF, if you use a frequency higher than 100Hz you may start to see some attenuation. At 6kHz you should see 50% of the input amplitude, for example.
  - _Fail_ “Audio IN or Right OUT not working” if signal doesn’t pass from Waveform In to Right OUT


When confirmed audio and LFO outs, press rotary to continue to next test.


###External ADC Chip Test
This is an automatic test and only stops if there’s a problem.

All lights turn off. The LFO LEDs and inner ring LED A (upper right) turn pure blue.

ADC chip 1 and 2 are automatically initialized.

  - _Fail_ “ADC chip 1 SPI fail” if slider A is blinking.
  - _Fail_ “ADC chip 2 SPI fail” if slider B is blinking.

The ADC channels are tested to verify the reading is close to 0 when no cable is patched into the jack. When a jack (ADC channel) passes the test, its light will turn off. 
If any 1V/oct jack ADC channel fails, then the corresponding LFO light will stay blue. If the Transpose jack ADC channel fails, the inner ring A light (upper right) will stay blue.

  - _Fail_ “1V/oct jack ## (or Transpose jack)” if one of the LFO or inner ring lights is staying blue.

If there’s an error, press rotary to continue.
Otherwise, if there’s no error then it automatically goes to the next test.

###Input Jacks Test
It will flash all LEDs green briefly while it sets up the internal ADCs to read the remaining CV jacks. 

Immediately, all green lights should turn off. If a green light is on, the ARM chip is not reading 0V on the corresponding jack. (Make sure no cables are patched)

  - _Fail_ “Internal ADC init error”: if it hangs with lights green. You must reboot after this, you cannot continue.


Check all the lights

  * LFO LEDs are flashing white
  * Clock In and Waveform In LEDs blue
  * Top row of buttons purple
  * LFO->VCA button blue
  * LFO Type button hot pink
  * 9 lights red: LFO CV, 5 on the inner ring, and the 3 LED rotaries
  * 1 light purple: Inner ring A (upper right)


###Input Jack Testing Procedure

1. Patch LFO A into Clk In jack: Clk In light should flash to tempo.
  * LFO->VCA button should turn off when jack is sensed as plugged in. 
  * Unplug and Clk In and LFO->VCA button lights should go off and stay off

2. Patch any cable into Waveform In jack: blue light should turn off and stay off
3. Plug LFO A into each CV jack, one at a time:
  * Each CV jack has a light associated with it. 
     - Channel 1V/oct jacks -> channel buttons at top
     - Transpose jack -> LFO Gate/Trig/Shape button.
     - Depth CV -> Depth rotary
     - Dispersion CV -> Inner ring D (lower left)
     - Browse CV -> Inner ring E (middle left)
     - Sphere CV -> Inner ring F (upper left)
     - LFO CV -> LFO CV light
     - Dispersion Pat. -> Inner ring C (lower right)
     - Latitude CV -> Latitude rotary
     - WT Spread CV (above Longitude knob) -> Longitude rotary
     - Spread CV (below Transpose) -> Inner ring B (middle right)

 * Watch the jack’s corresponding light when you patch, it should rapidly flash blue and turn off blue when you unpatch. It should also turn off red when a 5V signal is measured. When you unpatch, its light should stay off.

 * Watch the outer light ring when you patch, it will show the waveform.

      - For the non-1V/oct jacks, each of the 18 lights on the outer ring should be white at some moment.
      - _Fail_ “CV jack [name] bad range if it doens’t turn on all 18 lights (or 17 is ok) or if it sticks at light 18 or light 1 for more than a flicker.

      - For the 1V/oct and Transpose jacks, only the right half of the lights will be covered by the waveform. 
      - _Fail_ “1V/oct [channel#]/Transpose jack bad range” If it covers, more or less than 9 lights

4. Inner Ring light A (upper right) is the BUS CLOCK and BUS SELECT light.
  * Red will be on until it receives a clock pulse on the bus
  * Green will be on if the clock bus is stuck at high
  * Blue will be on if a pulse is received on the select bus
  * If you can’t test the Bus, then the light will be purple (red+blue) until it receives a pulse on either bus. _Fail_ “Clock Bus stuck high” If it’s white (red+blue+green)

  * If you can test the Buses, tap a QCD and the light will turn blue. _Fail_ “Clock Bus not receiving” if it stays purple, yellow or red.
  * If you can test the Select Bus, send any program change message (it happens automatically every few seconds with some modules). Blue light should turn off. _Fail_ “Select Bus not receiving” if the blue light won’t turn off.

When done testing every jack, every light should be off except for the 6 white flashing LFO OUT lights. Press the center rotary to continue.


###External Flash Chip Test
The final test is automatic and only pauses for user interaction if there is a failure.
Erase/Write/Read ext FLASH (sectors before Wavetables)

  - _Fail_ “External Flash init error” if it’s blinking slider D.
  - _Fail_ “External Flash read/write error” if it’s blinking slider C.
If there’s no error, it’ll just keep going.


###Success!!!!
You will see an animation of all the lights in red, green, and blue if there was success. 
The final step is to reboot. 
The first boot after hardware will take longer than normal because the wavetables are being copied to flash.