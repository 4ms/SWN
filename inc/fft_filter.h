#pragma once
#include <stm32f7xx.h>
#include "arm_math.h"

void do_cfft_lpf_512_f32(int16_t *inbuf,  int16_t *outbuf, float *cinbuf,float freq);
void do_fft_lpf_q15(q15_t *inbuf, q15_t *outbuf, q15_t *fftbuf, uint16_t bufsize, float freq);
void do_fft_shift_16(int16_t *inbuf, int16_t *outbuf, float *tmpbuf, int16_t shift);
