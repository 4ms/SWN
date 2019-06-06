# Spherical Wavetable Navigator

## Firmware updates

### Where to get updates
You can always download the latest updates at the [4ms Company SWN page] (https://4mscompany.com/SWN).

### How to tell what version you have
When the module powers on, the version is displayed with a red and blue light on the light ring. 
The red light on the inner rings shows the major version.
The blue light on the outer ring shows the minor version. For example if the version is 1.2 then the major version is 1, and the minor version is 2.

On the outer light ring, if you imagine it as a clock face, the light just to the right of 12:00 is 0. The next light going around clockwise is 1. Then next light clockwise is 2, etc. The inner ring counts the same but it starts at 1 (and goes to 6). If you would like a picture to help visualize this, see the [SWN User Manual](https://4mscompany.com/SWN/manual/SWN-manual-1.0.pdf), page 29.


### Beta Firmware v2.0 (SWN shows version 1.2)

#### New features:

  * __Export Sphere__: in Sphere Recording Mode, enable Monitoring by pressing the green button. Then tap the center knob to export the sphere out the audio jack.
     *  You can use this to transfer your sphere to another SWN by patching the output of your SWN to the other SWN which is set to record a Sphere. 
     *  Or you can use this to backup a Sphere by recording the audio with a WAV Recorder, sampler or other audio recording device.
     *  Or you can record onto a WAV Recorder or computer and then open the file in SphereEdit.
  * __Record single waveform__: Pressing Fine + Record in Sphere Recording Mode will record over the current waveform only.
  * __Channel Locks__: press Fine + any channel button to lock/unlock a channel. 
     * The button flashes when a channel is locked, and the channel's light(s) will flash on the light ring.
     * Locked channels will not respond to any global parameter changes.
     * Locked channels will still respond to individual channel parameter changes (holding the channel button and turning a knob)
  * __Sphere Selection display__: When changing the Sphere (push and turn center knob), the outer light ring will display which channels are assigned to which Spheres (similar to the Octave display). The channel buttons will also change to the color of the channel's Sphere. Turning the WT Spread knob also activates this display. Tapping the center knob will show this display, too (useful for visualing the signal on the Sphere CV or WT Spread CV jacks). 
  * __Load/Clear Sphere__: In Sphere Recording Mode, turning the Preset knob allows you to load and clear Spheres in the same way you load presets (tap and tap again to load; hold for 8 seconds and then tap to clear). Cleared Spheres can be uncleared by immediately doing the clear button press.
  * __Enable/Disable Sphere__: In Sphere Recording Mode, turn the Preset knob to select a Sphere. Tap the WT Select knob to disable the selected Sphere. The sphere's light on the outer ring will flash rapidly to show it's disabled. 
     * To re-enable the sphere, do the same thing again.
     * When you exit Sphere Recording mode, disabled Spheres will not show up when selecting Spheres or using WT Spread. 
     * The set of enabled/disabled Spheres is saved in the Presets. So you can enable/disable different sets of Spheres to be used with each Preset. 
  * __Reset Nav/Sphere__: Press Preset + Depth to reset navigation (browse/lat/long/depth). Press Preset + Latitude to reset Sphere selections. You can still press Preset + Depth + Latitude to reset both navigation and sphere selection (just like in version 1.0 and 1.1)
  * Octave range is now -3 to +14 (formerly was 0 to +14).


#### Bug fixes:

  * Bug where LFO Speeds would be set to super slow speeds if LFO CV jack goes from high to low rapidly.
  
   
#### Improvements:

  * Setting each channel to a different octave, and then turning global octave all the way up or down, and then back to the original position does not clear the intervals between channels (just like how Transpose has always worked)
  * WT Spread now offsets the channels in a more consistant way by adding increasing amounts of space between channels.
  * LED display tuned so that turning knobs always has an immediate effect, even if all CV jacks are being used with rapidly changing CV



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