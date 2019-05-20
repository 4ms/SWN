# Spherical Wavetable Navigator

## Firmware updates

### Where to get updates
You can always download the latest updates at the [4ms Company SWN page] (https://4mscompany.com/SWN).

### How to tell what version you have
When the module powers on, the version is displayed with a red and blue light on the light ring. 
The red light on the inner rings shows the major version.
The blue light on the outer ring shows the minor version. For example if the version is 1.2 then the major version is 1, and the minor version is 2.

On the outer light ring, if you imagine it as a clock face, the light just to the right of 12:00 is 0. The next light going around clockwise is 1. Then next light clockwise is 2, etc. The inner ring counts the same but it starts at 1 (and goes to 6). If you would like a picture to help visualize this, see the [SWN User Manual](https://4mscompany.com/SWN/manual/SWN-manual-1.0.pdf), page 29.




### Firmware v1.1
Released May 20, 2019

#### Bug fixes:

  * Fixed zippering sound when level changes rapidly (from internal or external LFO with sharp attack or decay).
  * Fixed position of white light on the outer light ring that shows which channels are not detuned. Bug only appeared when some channels were detuned and others were not detuned.
  * Fixed color of inner ring lights in Sphere Recording Mode for showing which waveform in the Sphere is selected. 
  * Fixed issue where loading a preset that contained a Scale would not refresh the channel pitches.
  
   
#### Improvements:

  * Softer distortion when clipping on the Waveform In jack
  * When entering Sphere Recording Mode, waveform 0,0,0 is selected and any Latitude, Longitude, or Depth offset is cleared. This ensures that Browsing will move through waveforms in the same order that they've been recorded.
  * When the SWN is showing the transpose/spread display, and mute buttons are pressed, the buttons now immediately show the new mute state.
  * Transpose/Spread display goes back to normal display mode more quickly.

  
### Firmware v1.0
  
Released May 3, 2019
  
First public version.