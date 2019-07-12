/*
 * math_util.c
 *
 * Author: Dan Green (danngreen1@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
 */

#include "math_util.h"
#include "math.h"
#include "gpio_pins.h" //debugging

//Clamp returns a value >=low and <=high
int8_t _CLAMP_I8(int8_t test, int8_t low, int8_t high)
{ 
		if (test<low)	return low;
		if (test>high) 	return high;
		return test;
}
int16_t _CLAMP_I16(int16_t test, int16_t low, int16_t high)
{ 
		if (test<low)	return low;
		if (test>high) 	return high;
		return test;
}
int32_t _CLAMP_I32(int32_t test, int32_t low, int32_t high)
{ 
		if (test<low)	return low;
		if (test>high) 	return high;
		return test;
}
uint8_t _CLAMP_U8(uint8_t test, uint8_t low, uint8_t high)
{
		if (test<low)	return low;
		if (test>high) 	return high;
		return test;
}
uint16_t _CLAMP_U16(uint16_t test, uint16_t low, uint16_t high)
{ 
		if (test<low)	return low;
		if (test>high) 	return high;
		return test;
}
uint32_t _CLAMP_U32(uint32_t test, uint32_t low, uint32_t high)
{ 
		if (test<low)	return low;
		if (test>high) 	return high;
		return test;
}
float _CLAMP_F(float test, float low, float high)
{
		if (test<low)	return low;
		if (test>high) 	return high;
		return test;
}

uint8_t _ABS_I8(int8_t a){
	return (a>0) ? a : ((int8_t)-1)*a;
}
uint16_t _ABS_I16(int16_t a){
	return (a>0) ? a : ((int16_t)-1)*a;
}
uint32_t _ABS_I32(int32_t a){
	return (a>0) ? a : ((int32_t)-1)*a;
}


//Fold turns a ramp from 0 to fold_point*2 into a triangle from 0 to fold_point to 0
//Max val of input must be fold_point * 2
//Min val of input must be 0
//Output will have min/max of 0 to fold_point 
float _FOLD_F(float val, float fold_point)
{
	val = _CLAMP_F(val, 0, fold_point*2);
	if (val > fold_point) val = 2*fold_point - val;
	return val;
}
uint16_t _FOLD_U16(uint16_t val, uint16_t fold_point)
{
	val = _CLAMP_U16(val, 0, fold_point*2);
	if (val > fold_point) val = 2*fold_point - val;
	return val;
}

//Scale: given a value and a range for the value, return a linearly scaled value in another given range
//
uint16_t _SCALE_F2U16(float in, float in_min, float in_max, uint16_t out_min, uint16_t out_max)
{
	return (  (uint16_t)( ((in-in_min)/(in_max - in_min)) * (float)(out_max-out_min)) + out_min  );
}
float _SCALE_U2F(uint16_t in, uint16_t in_min, uint16_t in_max, float out_min, float out_max)
{
	return (   (  ( ((float)(in-in_min)) / ((float)(in_max - in_min)) ) * (out_max-out_min)) + out_min  );
}
uint32_t _SCALE_U2U(uint32_t in, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max)
{
	return (   (  ( ((float)(in-in_min)) / ((float)(in_max - in_min)) ) * (float)(out_max-out_min)) + (float)out_min  );
}

//Wrap returns a value: min <= val < max
//if max>min, returns 0
uint8_t	_WRAP_U8(uint8_t val, uint8_t min, uint8_t max)
{
	if (max<=min) return 0; //error: bad parameters
	while (val<min) val+=(max-min);
	while (val>=max) val-=(max-min);
	return val;
}
uint32_t _WRAP_U32(uint32_t val, uint32_t min, uint32_t max)
{
	if (max<=min) return 0; //error: bad parameters
	while (val<min) val+=(max-min);
	while (val>=max) val-=(max-min);
	return val;
}
uint16_t _WRAP_U16(uint16_t val, uint16_t min, uint16_t max)
{
	if (max<=min) return 0; //error: bad parameters
	while (val<min) val+=(max-min);
	while (val>=max) val-=(max-min);
	return val;
}
int8_t	_WRAP_I8(int8_t val, int8_t min, int8_t max)
{
	if (max<=min) return 0; //error: bad parameters
	while (val<min) val+=(max-min);
	while (val>=max) val-=(max-min);
	return val;
}
int16_t	_WRAP_I16(int16_t val, int16_t min, int16_t max)
{
	if (max<=min) return 0; //error: bad parameters
	while (val<min) val+=(max-min);
	while (val>=max) val-=(max-min);
	return val;
}
int32_t	_WRAP_I32(int32_t val, int32_t min, int32_t max)
{
	if (max<=min) return 0; //error: bad parameters
	while (val<min) val+=(max-min);
	while (val>=max) val-=(max-min);
	return val;
}
float _WRAP_F(float val, float min, float max)
{
	if (max<=min) return 0; //error: bad parameters
	while (val<min) val+=(max-min);
	while (val>=max) val-=(max-min);
	return val;
}

//xfade=0 	===> a
//xfade=0.5 ===> (a+b)/2
//xfade=1 	===> b
// float _CROSSFADE(float a, float b, float xfade)
// {
// 	return (a*(1.0-xfade)) + (b*xfade);
// }

//56.28, 0.1 >> 56.2
float _QUANTIZE_F(float a, float qnt)
{
	if (a>INT32_MAX || a<INT32_MIN) return a; //can't handle it

	int32_t scaled_int = (a/qnt);
	return (float)scaled_int*qnt;
}


const float TWELFTH_ROOTS_TWO[12] = {
	1.0,
	1.0594630943593,
	1.12246204830937,
	1.18920711500272,
	1.25992104989487,
	1.33483985417003,
	1.4142135623731,
	1.49830707687668,
	1.5874010519682,
	1.68179283050743,
	1.78179743628068,
	1.88774862536339
};

// Finds the n/12th root of 2, (cheaply) linearly crossfading between integer values of n
float _N_OVER_12TH_ROOT_TWO(float n)
{
	uint32_t i_n = (uint32_t)n;
	float root = 1.0;

	while (i_n>=48) {
		root *= 16.0;
		i_n-=48;
	}
	if (i_n>=36) {
		root *= 8.0;
		i_n-=36;
	}
	if (i_n>=24) {
		root *= 4.0;
		i_n-=24;
	}
	if (i_n>=12) {
		root *= 2.0;
		i_n-=12;
	}
	root *= TWELFTH_ROOTS_TWO[i_n];

	root *= _CROSSFADE(1.0, TWELFTH_ROOT_2, n - (uint32_t)n);

	return root;
}


// Given an array of floats, tosses out the min and max values 
// and returns the average of the remaining elements
//
float _AVERAGE_EXCL_MINMAX_F(float *lpf_values, uint32_t num_elements)
{
	uint32_t i;
	float max=-HUGE_VALF, min=HUGE_VALF;
	float sum=0;

	if (num_elements==0) return 0;
	if (num_elements==1) return lpf_values[0];
	if (num_elements==2) return (lpf_values[0] + lpf_values[1])/2;

	for (i=0;i<num_elements;i++)
	{
		if (lpf_values[i] > max) max = lpf_values[i];
		if (lpf_values[i] < min) min = lpf_values[i];
		sum+=lpf_values[i];
	}
	sum = sum - max - min;
	num_elements-=2;

	return ((float)sum)/((float)num_elements);
}
