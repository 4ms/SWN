#wavecalc
## Wavetable calculator for wavetable oscillator
Authors: Hugo Paris hugoplho@gmail.com, Dan Green danngreen1@gmail.com

Usage: 

`wavecalc [input_wav_dir [output_dir [author]]]`

The spherename will be extracted from `input_wav_dir`.
The output will be put into `output_dir/(spherename).h`

`input_wav_dir` should contain 27 .wav files.

After running wavecalc:

1) Copy the .h file into the SWN folder inc/spheres/

2) Add the following line to the top of `SWN_PROJECT_DIR/inc/spheres_internal.h`:

`#include "spheres/spherename.h"`

3) Add `(void *)spherename` inside the declaration for `const void *wavetable_list[]={...}` in `spheres_internal.h`

