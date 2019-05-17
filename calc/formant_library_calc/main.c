#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/malloc.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <float.h>
#include <dirent.h> 

#include "wavfile.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846 
#endif


// MAIN OPTIONS
int 	UPDATE_WAVETABLES 	= 	1;
#define TABLELEN 				512 		// use multiples of 4
#define OCTDROP					8

char *	FORMAT				=	"8bit"; 	// 8bit, 16bit, float
int 	transose_amt 		=   1<<OCTDROP;


// GLOBAL
#define NUM_VOWELS  			27	
#define NUM_FORMANTS  			5	
#define	SR 						16742.4 / 8 //16742.4			// Sanmple Rate
#define LPF						0.05	
#define MAX8BITS  				127
#define MAX16BITS 				32767
#define MAX_INDENT 				10 		// segmentation 11 -> check this number
#define HEADROOM				0.01		


// // WAVEFORMS
// float data_lpf;
// float data_down[TABLELEN];

// // WAVE-FILES
// #define HEADERLEN 	312
// // char *wavefolder = "../libraries/wavefiles/SelectionB/";
// char *wavefolder = "../libraries/wavefiles/Full_Library/";

	
// // WAVE-FILE HEADER FORMAT
// struct HEADER {
//     unsigned char 	riff[4];                    // RIFF string
//     unsigned int 	overall_size   ;            // overall size of file in bytes
//     unsigned char 	wave[4];                    // WAVE string
//     unsigned char 	fmt_chunk_marker[4];        // fmt string with trailing null char
//     unsigned int 	length_of_fmt;              // length of the format data
//     unsigned int 	format_type;                // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
//     unsigned int 	channels;                   // no.of channels
//     unsigned int 	sample_rate;                // sampling rate (blocks per second)
//     unsigned int 	byterate;                   // SampleRate * NumChannels * BitsPerSample/8
//     unsigned int 	block_align;                // NumChannels * BitsPerSample/8
//     unsigned int 	bits_per_sample;            // bits per sample, 8- 8bits, 16- 16 bits etc
//     unsigned char 	data_chunk_header [4];      // DATA string or FLLR string
//     unsigned int 	data_size;                  // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
// };


// struct HEADER header[10000]; 					// need at least as many as number of waveforms
// unsigned char buffer4[4];
// unsigned char buffer2[2];


// // BASIC WAVEFORMS
// char *Basic_Waveforms[] = { 					// Array of pointers
// 	"SAW",	
// 	"TRIANGLE",
// 	"SQUARE",
// 	"SINE"
// 	};


// // WAVEFORMS FROM WAV-FILES
// int32_t temp_wav[TABLELEN];
// long 	wavebuf[TABLELEN];


// // WAVETABLE STRUCTURE
// float	full_data[TABLELEN];
float	data[TABLELEN];
float	filter_out[5][TABLELEN];
float	triangle[TABLELEN];
float	saw[TABLELEN];
float 	square[TABLELEN];
float	noise[TABLELEN];
float	source[TABLELEN];
float  	temp[TABLELEN];
// float	data_rev[TABLELEN];
// float	data_rev[TABLELEN];
// char* 	name;
// char*  	name_rev;	


// FUNCTIONS
// Random numbers
float gaussrand();


// waveform to library
void print_waveform_to_library(char* name, float waveform[TABLELEN], char* format, int tablelen, int reverse);
// void UPDATE_WAVETABLES_library(char* name, float waveform[TABLELEN], char* format, int update);
void plot_wavetable(char plot_title[1024], float waveform[TABLELEN], int tablelen, int color);

void write_waveform_to_wav(char* name, float waveform[TABLELEN], int tablelen);

// character manipulations
char* concat(const char *s1, const char *s2);
char *replace(const char *s, char ch, const char *repl); 


// void plot_wavetable(char plot_title[1024],int dof, float waveform[NUM_WAV_WAVEFORMS]);

int main(void){
	int i, ii, c;
	uint32_t j;
	int 		k 		= 0;
	int 		l 		= 0;
	int 		dof 	= 0; 			// degree of freedom: x,y,z
	int 		read 	= 0;
	float 		inc, head;	
	float 		sum_buf;
	float 		maxval_buf;
	float 		posmaxvalue, negmaxvalue;
	char 		filename[1000];
	DIR 		*d;
	struct 		dirent 	*dir;
	float		modulo;
	uint32_t 	waveform_clipped=0;
	char i_str[35];

	
	printf("\n*********************************\n FORMANT FILTERS \n*********************************\n\n");

	// // X-loop: filling up one row
	// // Y-loop: then filling up another row on Y+=1 until columns are complete
	// // Z-loop: move on to next layer (z+=1) until all layers are complete
	// int cx=0;
	// int cy=0;
	// int cz=0;

	// uint8_t OCTUP;
	// if(OCTDROP>3) {OCTUP =	OCTDROP - 3;}
	// else 		  {OCTUP =	0;}

	// FIXME: ADD BACKUP BEFORE MOVING
		
	// // REMOVE PREVIOUS WAVEFORM FILES FROM FOLDER
	// if 		(TABLELEN == 256) {system("exec rm -r ../libraries/waveform/256*.h" );}
	// else if (TABLELEN == 512) {system("exec rm -r ../libraries/waveform/512*.h" );}
	// else if (TABLELEN == 1024){system("exec rm -r ../libraries/waveform/1024*.h");}

	// TRIANGLE 
	inc = 1.0f/TABLELEN; // * (1<<OCTUP)/ TABLELEN ;
	modulo = 0.75;
	for (i=0; i<TABLELEN; i++){
		if(modulo>1){modulo-=1;}
		triangle[i] = 2 * fabsf (2 * modulo - 1) -1;
		modulo += inc;			
	}
	// plot_wavetable("triangle", triangle, TABLELEN, 1);	

	// SAW
	inc = 1.0f/TABLELEN;// * (1<<OCTUP)/ TABLELEN ;
	temp[0]  = 0;
	saw[0] 	 = 1;
	for (j=1; j < TABLELEN+1; j++){
		temp[j]   = temp[j-1] + inc;
			if (temp[j]>  1){temp[j]=0 ;}
		saw[j] = 2*(1 - fabsf(temp[j]))-1; 
	}
	// plot_wavetable("saw", saw, TABLELEN, 1);	

	// SQUARE
	inc = 1.0f/TABLELEN; //(float)(1<<OCTUP) / TABLELEN;
	modulo = 0;
	for (j=0; j < TABLELEN; j++){
		if (modulo>1){modulo-=1;}
		if 		( modulo<=0.5)     				{square[j]=1;}
		else if ((modulo>0.5 ) && (modulo <=1)) {square[j]=0;}
		modulo += inc;
	}	
	// plot_wavetable("square", square, TABLELEN, 1);	

	// NOISE (Gaussian distribution) 
	for (j=0; j < TABLELEN+1; j++){
		noise[j] = gaussrand();
	}
	// normalize
	maxval_buf  = 0;
	for (j=0; j<TABLELEN; j++){
		if (fabsf(noise[j]) > maxval_buf){
			maxval_buf = fabsf(noise[j]);
		} 
	}
	for (j=0; j<TABLELEN; j++){
		noise[j] /= (float)(maxval_buf);
	}	
	// plot_wavetable("noise", noise, TABLELEN, 1);	


	// FORMANT SYNTHESIS INPUT
	for (i=0; i<TABLELEN; i++){
		source[i]= 0.3 * saw[i] + 0.7 * noise[i]; 
		// source[i]= noise[i]; 		
	}
	// plot_wavetable("Source", source, TABLELEN, 1);	


	

	// FIXME: Add white noise to mix in before formant filter

	// FORMANT FILTERS
	// coefficients by HTML Csound Manual - © Jean Piché & Peter J. Nix, 1994-97 
	// https://www.classes.cs.uchicago.edu/archive/1999/spring/CS295/Computing_Resources/Csound/CsManual3.48b1.HTML/Appendices/table3.html
	float A[5][NUM_VOWELS], B[5][NUM_VOWELS], C[5][NUM_VOWELS];

	typedef struct $
	{
		// name
		char name[100];

		// Freq
		uint16_t	F0;
		uint16_t	F1;
		uint16_t	F2;
		uint16_t	F3;
		uint16_t	F4;
		
		// Bandwidth
		uint16_t	BW0;
		uint16_t	BW1;
		uint16_t	BW2;
		uint16_t	BW3;
		uint16_t	BW4;

		// Gain
		float		G0;
		float		G1;
		float		G2;
		float		G3;
		float		G4;

	} formant_filter;

	formant_filter formant[NUM_VOWELS] = {
		// NAME 				Freq (Hz) 													Bandwidth 											Gain	
		{"soprano a" ,			800,		1150,		2900,		3900,		4950,		80,		90,		120,		130,		140,		1,			0.501187234,			0.025118864,			0.1,					0.003162278 },
		{"soprano e" ,			350,		2000,		2800,		3600,		4950,		60,		100,	120,		150,		200,		1,			0.1,					0.177827941,			0.01,					0.001584893 },
		{"soprano i" ,			270,		2140,		2950,		3900,		4950,		60,		90,		100,		120,		120,		1,			0.251188643,			0.050118723,			0.050118723,			0.006309573 },
		{"soprano o" ,			450,		800,		2830,		3800,		4950,		70,		80,		100,		130,		135,		1,			0.281838293,			0.079432823,			0.079432823,			0.003162278 },
		{"soprano u" ,			325,		700,		2700,		3800,		4950,		50,		60,		170,		180,		200,		1,			0.158489319,			0.017782794,			0.01,					0.001		},
		{"alto a" ,				800,		1150,		2800,		3500,		4950,		80,		90,		120,		130,		140,		1,			0.630957344,			0.1,					0.015848932,			0.001		},
		{"alto e" ,				400,		1600,		2700,		3300,		4950,		60,		80,		120,		150,		200,		1,			0.063095734,			0.031622777,			0.017782794,			0.001		},
		{"alto i" ,				350,		1700,		2700,		3700,		4950,		50,		100,	120,		150,		200,		1,			0.1,					0.031622777,			0.015848932,			0.001		},
		{"alto o" ,				450,		800,		2830,		3500,		4950,		70,		80,		100,		130,		135,		1,			0.354813389,			0.158489319,			0.039810717,			0.001778279	},
		{"alto u" ,				325,		700,		2530,		3500,		4950,		50,		60,		170,		180,		200,		1,			0.251188643,			0.031622777,			0.01,					0.000630957	},
		{"countertenor a" ,		660,		1120,		2750,		3000,		3350,		80,		90,		120,		130,		140,		1,			0.501187234,			0.070794578,			0.063095734,			0.012589254	},
		{"countertenor e" ,		440,		1800,		2700,		3000,		3300,		70,		80,		100,		120,		120,		1,			0.199526231,			0.125892541,			0.1,					0.1			},
		{"countertenor i" ,		270,		1850,		2900,		3350,		3590,		40,		90,		100,		120,		120,		1,			0.063095734,			0.063095734,			0.015848932,			0.015848932	},
		{"countertenor o" ,		430,		820,		2700,		3000,		3300,		40,		80,		100,		120,		120,		1,			0.316227766,			0.050118723,			0.079432823,			0.019952623	},
		{"countertenor u" ,		370,		630,		2750,		3000,		3400,		40,		60,		100,		120,		120,		1,			0.1,					0.070794578,			0.031622777,			0.019952623	},
		{"tenor a" ,			650,		1080,		2650,		2900,		3250,		80,		90,		120,		130,		140,		1,			0.501187234,			0.446683592,			0.398107171,			0.079432823	},
		{"tenor e" ,			400,		1700,		2600,		3200,		3580,		70,		80,		100,		120,		120,		1,			0.199526231,			0.251188643,			0.199526231,			0.1			},
		{"tenor i" ,			290,		1870,		2800,		3250,		3540,		40,		90,		100,		120,		120,		1,			0.177827941,			0.125892541,			0.1,					0.031622777	},
		{"tenor o" ,			400,		800,		2600,		2800,		3000,		40,		80,		100,		120,		120,		1,			0.316227766,			0.251188643,			0.251188643,			0.050118723	},
		{"tenor u" ,			350,		600,		2700,		2900,		3300,		40,		60,		100,		120,		120,		1,			0.1,					0.141253754,			0.199526231,			0.050118723	},
		{"bass  a" ,			600,		1040,		2250,		2450,		2750,		60,		70,		110,		120,		130,		1,			0.446683592,			0.354813389,			0.354813389,			0.1			},
		{"bass  e" ,			400,		1620,		2400,		2800,		3100,		40,		80,		100,		120,		120,		1,			0.251188643,			0.354813389,			0.251188643,			0.125892541	},
		{"bass  i" ,			250,		1750,		2600,		3050,		3340,		60,		90,		100,		120,		120,		1,			0.031622777,			0.158489319,			0.079432823,			0.039810717	},
		{"bass  o" ,			400,		750,		2400,		2600,		2900,		40,		80,		100,		120,		120,		1,			0.281838293,			0.089125094,			0.1,					0.01		},
		{"bass  u" ,			350,		600,		2400,		2675,		2950,		40,		80,		100,		120,		120,		1,			0.1,					0.025118864,			0.039810717,			0.015848932	}, 

		{"exp  a" ,				560,		1000,		2150,		2000,		2250,		35,		70,		80,			100,		100,		1,			0.501187234,			0.070794578,			0.063095734,			0.012589254	},
		{"exp  e" ,				380,		1450,		2300,		2600,		2800,		35,		70,		80,			100,		100,		1,			0.199526231,			0.125892541,			0.1,					0.1			}
	};


	for(i=0; i<NUM_VOWELS; i++){

		// Display filter caracteristics
		printf("%s:\n",				formant[i].name);
		printf("\tF0: %u \n", 	 	formant[i].F0);		
		printf("\tF1: %u \n", 		formant[i].F1);		
		printf("\tF2: %u \n", 	 	formant[i].F2);		
		printf("\tF3: %u \n", 	 	formant[i].F3);		
		printf("\tF4: %u \n", 	 	formant[i].F4);		
		printf("\tBW0: %u \n", 		formant[i].BW0);		
		printf("\tBW1: %u \n", 		formant[i].BW1);		
		printf("\tBW2: %u \n", 		formant[i].BW2);		
		printf("\tBW3: %u \n", 		formant[i].BW3);		
		printf("\tBW4: %u \n\n", 	formant[i].BW4);		

		// Compute coefficients
		C[0][i] =  -1 * expf(-2 * M_PI 	* formant[i].BW0 / SR);
		B[0][i] =  (2 * expf(-1 * M_PI 	* formant[i].BW0 / SR)) * cos(2 * M_PI * formant[i].F0 / SR);
		A[0][i] =  1 - C[0][i] - B[0][i];

		C[1][i] =  -1 * expf(-2 * M_PI 	* formant[i].BW1 / SR);
		B[1][i] =  (2 * expf(-1 * M_PI 	* formant[i].BW1 / SR)) * cos(2 * M_PI * formant[i].F1 / SR);
		A[1][i] =  1 - C[1][i] - B[1][i];

		C[2][i] =  -1 * expf(-2 * M_PI 	* formant[i].BW2 / SR);
		B[2][i] =  (2 * expf(-1 * M_PI 	* formant[i].BW2 / SR)) * cos(2 * M_PI * formant[i].F2 / SR);
		A[2][i] =  1 - C[2][i] - B[2][i];

		C[3][i] =  -1 * expf(-2 * M_PI 	* formant[i].BW3 / SR);
		B[3][i] =  (2 * expf(-1 * M_PI 	* formant[i].BW3 / SR)) * cos(2 * M_PI * formant[i].F3 / SR);
		A[3][i] =  1 - C[3][i] - B[3][i];

		C[4][i] =  -1 * expf(-2 * M_PI 	* formant[i].BW4 / SR);
		B[4][i] =  (2 * expf(-1 * M_PI 	* formant[i].BW4 / SR)) * cos(2 * M_PI * formant[i].F4 / SR);
		A[4][i] =  1 - C[4][i] - B[4][i];

		// Display Coefficients
		if(0){
			printf("\tC0: %f\n",		C[0][i]);
			printf("\tB0: %f\n",		B[0][i]);
			printf("\tA0: %f\n",		A[0][i]);
			printf("\tC1: %f\n",		C[1][i]);
			printf("\tB1: %f\n",		B[1][i]);
			printf("\tA1: %f\n",		A[1][i]);
			printf("\tC2: %f\n",		C[2][i]);
			printf("\tB2: %f\n",		B[2][i]);
			printf("\tA2: %f\n",		A[2][i]);
			printf("\tC3: %f\n",		C[3][i]);
			printf("\tB3: %f\n",		B[3][i]);
			printf("\tA3: %f\n",		A[3][i]);
			printf("\tC4: %f\n",		C[4][i]);
			printf("\tB4: %f\n",		B[4][i]);
			printf("\tA4: %f\n",		A[4][i]);
		}

		// Compute individual filter outputs
		// y[n] = A*x[n] + B*y[n-1] + C*y[n-2]
		for (k=0; k<NUM_FORMANTS; k++){
			// re-run filter to get rid of effect of initial conditions
			// ... on first iterations
			filter_out[k][TABLELEN-1] = 0;
			filter_out[k][TABLELEN-2] = 0;
			for (l=0; l<3; l++) {
				for (j=0; j<TABLELEN; j++) {
					if 		(j==0){filter_out[k][j] = (A[k][i] * source[j]) + (B[k][i] * filter_out[k][TABLELEN-1])  + (C[k][i] * filter_out[k][TABLELEN-2]);}
					else if (j==1){filter_out[k][j] = (A[k][i] * source[j]) + (B[k][i] * filter_out[k][j-1]) 		 + (C[k][i] * filter_out[k][TABLELEN-1]);}
					else{		   filter_out[k][j] = (A[k][i] * source[j]) + (B[k][i] * filter_out[k][j-1]) 		 + (C[k][i] * filter_out[k][j-2]);}
				}
			}

			//Smooth the transistion/seam
			for (j=0; j<20; j++) {
				filter_out[k][TABLELEN-20+j] = (filter_out[k][TABLELEN-20+j] * (20-j)/20.0) + (filter_out[k][0] * j/20.0);
			}
		}


		// Sum Formants in parallel
		// ... to avoid quantization-noise
		// ... theoretically in series
		// FIXME: add filter gain
		for (j=0; j<TABLELEN; j++){
			data[j] = 	  formant[i].G0 * filter_out[0][j]
						+ formant[i].G1 * filter_out[1][j]
						+ formant[i].G2 * filter_out[2][j]
						+ formant[i].G3 * filter_out[3][j]
						+ formant[i].G4 * filter_out[4][j];
		}

		// // DETECT DC OFFSET
		// sum_buf=0;
		// for (j=0; j<TABLELEN; j++){
		// 	sum_buf += data[j];
		// }

		// // REMOVE DC OFFSET
		// for (j=0; j<TABLELEN; j++){
		// 	data[j] -= (sum_buf/TABLELEN);
		// }

		// FIND MAX AMPLITUDE
		maxval_buf  = 0;
		for (j=0; j<TABLELEN; j++){
			if (fabsf(data[j]) > maxval_buf){
				maxval_buf = fabsf(data[j]);
			} 
		}

		// NORMALIZE WAVEFORM GAIN 
		for (j=0; j<TABLELEN; j++){
			data[j] /= (float)(maxval_buf);
		}	

		// PLOT WAVEFORM
		// plot_wavetable(formant[i].name, data, TABLELEN, 1);	

		// PRINT WAVEFORM TO LIBRARY
		//print_waveform_to_library(formant[i].name, data, "float", TABLELEN, 0);
		print_waveform_to_library(formant[i].name, data, "16bit", TABLELEN, 0);
		//print_waveform_to_library(formant[i].name, data, "8bit",  TABLELEN, 0);
		//UPDATE_WAVETABLES_library(formant[i].name, data, "16bit", UPDATE_WAVETABLES);										
		// UPDATE_WAVETABLES_library(formant[i].name, data, "8bit",  UPDATE_WAVETABLES);	

		printf("writing vowel into ../libraries/formants/wavs/%02d.wav", i);
		printf("\n");

		sprintf(i_str, "../libraries/formants/wavs/%02d.wav", i);
		
		write_waveform_to_wav(i_str, data, TABLELEN);
	}


// y[n] = A*x[n] + B*y[n-1] + C*y[n-2]


	// FILTER #2
	
	// FILTER #3



	
		
	
	// // MAKE COSINE WINDOW
	// float cosine_window[TABLELEN];
	// for (i=0; i<TABLELEN; i++){
	// 	cosine_window[i] = sin(i*M_PI/(TABLELEN-1)); 
	// 	cosine_window[i] *= cosine_window[i]; 
	// }
	
	// //WIDEN COSINE WINDOW
	// int cosine_gain = 100;
	// for (i=0; i<TABLELEN; i++){
	// 	cosine_window[i] *= cosine_gain;
	// 	if (cosine_window[i] > 1){cosine_window[i]=1;}
	// }

/*
	// PLOT WINDOW
						
		//Write data to temporary file
		FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
		for (i=0; i < TABLELEN; i++){
			fprintf(tempb, "%d %lf \n", i, cosine_window[i]); 
		}
	
		//Send commands to gnuplot one by one.
		for (i=0; i < NUM_COMMANDS; i++){
			fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); 
		}
*/
			
		
	// // TRANSPOSING DOWN
	// if (OCTDROP){
	// 	for (i=0; i< TABLELEN-1; i++){
	// 		if (j == 0){
	// 			data_down[i] = full_data[i];
	// 			data_buffer = data_down[i];
	// 		} else {
	// 			data_down[i] =  data_buffer + j * (data_down[i+1]-data_buffer)/transose_amt;
	// 		}
	// 		j++;
	// 		if (j>transose_amt){j=0;}
	// 	}
	// }
		
	// // LPF
	// data_lpf = 0;
	// for (i=0; i<TABLELEN; i++){
	// 	data_lpf *= 1 - LPF;
	// 	data_lpf += LPF * data_down[i];
	// 	data[i] = data_lpf;
	// }				

	
	
	// SMOOTHING:
	// LINEAR INTERPOLATION FOR NEAR-ZERO SAMPLES
// 				for (j=0; j<10; j++){
// 					for (i=0; i<TABLELEN; i++){
// 						if (((fabsf(data[i]))<1000)&&
// 							(i!=0) && (i<TABLELEN-1)){
// 							data[i] = (data[i-1] + data[i+1])/2;
// 						}
// 					}	
// 				}

							



// 	// APPLY WINDOW
// 	for (i=0; i<TABLELEN; i++){
// 		data[i] *= cosine_window[i];
// 	}

	
// 	// FIND MAX AMPLITUDE
// 	maxval_buf  = 0;
// 	for (i=0; i<TABLELEN; i++){
// 		if (fabsf(data[i]) > maxval_buf){
// 			maxval_buf = fabsf(data[i]);
// 		} 
// 	}
	

// 	// NORMALIZE WAVEFORM GAIN 
// 	for (i=0; i<TABLELEN; i++){
// 		data[i] /= (float)(maxval_buf);
// 	}	
// 	printf("- Normalization applied\n");


// 	// FIND MAX POS/NEG AMPLITUDES
// 	posmaxvalue = 0;
// 	negmaxvalue = 0;
// 	for (i=0; i<TABLELEN; i++){
// 		if (data[i] > posmaxvalue) {
// 			posmaxvalue = data[i];
// 		}
// 		else if ((data[i]< 0) && (fabs(data[i]) > negmaxvalue)) {
// 			negmaxvalue = fabsf(data[i]);
// 		}
// 	}
// 	printf("- max positive amplitude: %f\n",posmaxvalue);
// 	printf("- max negative amplitude: %f\n",negmaxvalue);


// 	// ADJUST WAVEFORM GAIN ON WEAKER HALF" 
// 	for (i=0; i<TABLELEN; i++){
// 		if	   ((posmaxvalue > negmaxvalue) && (data[i]<0)){				
// 			data[i] *= posmaxvalue / negmaxvalue;
// 		}
// 		else if((posmaxvalue < negmaxvalue) && (data[i]>0)){				
// 			data[i] *= negmaxvalue / posmaxvalue;
// 		}
// 	}	
// 	printf("- weaker half adjusted\n");
	
// 	// LEAVE HEADROOM				
// 	for (i=0; i<TABLELEN; i++){
// 		// apply headroom
// 		// clip rest
// 		data[i] *= (1-HEADROOM);
// 		if 		(data[i] >  1){data[i]= 1;}
// 		else if (data[i] < -1){data[i]=-1;}					
// 	}
	
		
// 	// FIND MAX POS/NEG AMPLITUDES (SANITY CHECK)
// 	posmaxvalue = 0;
// 	negmaxvalue = 0;
// 	for (i=0; i<TABLELEN; i++){
// 		if (data[i] > posmaxvalue) {
// 			posmaxvalue = data[i];
// 		}
// 		else if ((data[i]< 0) && (fabs(data[i]) > negmaxvalue)) {
// 			negmaxvalue = fabsf(data[i]);
// 		}
// 	}
// 	printf("- max positive amplitude: %f\n",posmaxvalue);
// 	printf("- max negative amplitude: %f\n",negmaxvalue);
	
// 	if ((posmaxvalue>1) || (negmaxvalue< -1)){
// 		waveform_clipped +=1;
// 	}

// 	// PLOT WAVEFORM
// // 				plot_wavetable(name, data, TABLELEN, 3);
// 	printf("- Length of data: %d\n", (int)( sizeof(data) / sizeof(data[0]) ));


// 	// PRINT WAVEFORM TO LIBRARY
// 	print_waveform_to_library(name, data, "float", TABLELEN, 0);
// 	print_waveform_to_library(name, data, "16bit", TABLELEN, 0);
// 	print_waveform_to_library(name, data, "8bit",  TABLELEN, 0);
// 	UPDATE_WAVETABLES_library(name, data, FORMAT, UPDATE_WAVETABLES);										

	
// 	// ##################
// 	//  REVERSE WAVEFORM
// 	// ##################


// 	// NAME
// 	name_rev = "REVERSE_";
// 	name_rev = concat (name_rev, name); 
	

// 	// DATA
// 	for (i=1; i<=TABLELEN; i++){
// 		data_rev[i-1] = data[TABLELEN-i];					
// 	}					


// 	// PRINT REVERSE WAVEFORM TO LIBRARY		
// 	print_waveform_to_library(name_rev, data_rev, "float", TABLELEN, 1);
// 	print_waveform_to_library(name_rev, data_rev, "16bit", TABLELEN, 1);
// 	print_waveform_to_library(name_rev, data_rev, "8bit",  TABLELEN, 1);
// 	UPDATE_WAVETABLES_library(name_rev, data_rev, FORMAT, UPDATE_WAVETABLES);										


// 	// RESET MAX VALUE
// 	maxval_buf=0;	


// 	// FREE UP MEMORY
// 	fclose(ptr);						

 
}
	
	





//---------- FUNCTIONS ---------


void print_waveform_to_library(char* name, float waveform[TABLELEN], char* format, int tablelen, int reverse)
{
	uint32_t 	i;
	uint8_t 	charcount;
	uint8_t 	indent;
	char  		table_lenght[45];
	char* 		filename;
//     int 		old_stdout = dup(1);  
		
	// LIBRARY FILE
	if (tablelen == 1024)	{filename 	= concat("../libraries/formants/", "1024");};
	if (tablelen == 512)	{filename 	= concat("../libraries/formants/", "512");};
	if (tablelen == 256)	{filename	= concat("../libraries/formants/", "256");};
	if(reverse){
		filename = concat(filename,"_BWD");
	}else{
		filename = concat(filename,"_FWD");
	}
	filename = concat(filename,"_");
	filename = concat(filename, format);
	filename = concat(filename,"_waveform_library.h");
 	FILE *stream = fopen(filename, "a");

	// DATA
		// NAME
		fprintf(stream, "{{\"%s\"} , {", name);


	
		// WAVEFORM DATA
		
		// float
		if(strcmp(format,"float")==0){
			for (i=0; i<TABLELEN-1; i++){
				fprintf(stream, "%f, ",waveform[i]);
			}
			fprintf(stream, "%f},\n",waveform[TABLELEN-1]);	
		
		// 16bit	
		}else if(strcmp(format,"16bit")==0){
			for (i=0; i<TABLELEN-1; i++){
				fprintf(stream, "%d, ", (int16_t)(waveform[i]* (float)(MAX16BITS)) );
			}
			fprintf(stream, "%d}},\n",(int16_t)(waveform[TABLELEN-1]* (float)(MAX16BITS)));	
		
		// 8bit
		}else if(strcmp(format,"8bit")==0){
			for (i=0; i<TABLELEN-1; i++){
				fprintf(stream, "%d, ", (int8_t)(waveform[i]* (float)(MAX8BITS)) );
			}
			fprintf(stream, "%d},\n",(int8_t)(waveform[TABLELEN-1]* (float)(MAX8BITS)));	
		}
			
	fclose(stream);	
	free(filename);
}




void write_waveform_to_wav(char* name, float waveform[TABLELEN], int tablelen)
{
	uint32_t 	i;
	char 		*filename;
	
	int bitsPerSample = 16;
	int numChannels = 1;
	int recordingRate = 44100;

	WaveHeaderAndChunk whac;

	// filename = concat("../libraries/waveform/wavs/", name);
	// filename = concat(filename,".wav");

 	FILE *stream = fopen(name, "wb");

	create_waveheader(&whac, bitsPerSample, numChannels, recordingRate, tablelen);

	fwrite(&whac, 1, sizeof(WaveHeaderAndChunk), stream);

	int16_t samples[TABLELEN];

	for (i=0; i<TABLELEN; i++){
		samples[i] = (int16_t)(waveform[i]*32767.0);
	}

	fwrite(samples, TABLELEN, sizeof(int16_t), stream);
				
	fclose(stream);	
	// free(filename);
}

void UPDATE_WAVETABLES_library(char* name, float waveform[TABLELEN], char* format, int update)
{
	if(update){
		FILE * fp;
		char * line = NULL;
		size_t len = 0;
		ssize_t read;
		char* filename;
		char* temp_filename;
		char* replacement_line = "blahblah";
		char* command;
		char* command_copy;
		char* num_buf;
		uint32_t i,j;
		uint8_t charcount;
		uint8_t indent;
		char value_buf[200];
	
		// OPEN WAVETABLE FILE
		 filename = concat( "../libraries/wavetable/", "wavetable_struct.c");
// 		if (strcmp(format, "float") == 0){
// 			 filename = concat( "../libraries/wavetable/float/", "wavetable_struct.c");
// 		} else if (strcmp(format, "16bit") == 0){
// 			filename = concat( "../libraries/wavetable/16bit/", "wavetable_struct_16bit.c");
// 		} else if (strcmp(format, "8bit") == 0){
// 			filename = concat( "../libraries/wavetable/16bit/", "wavetable_struct_8bit.c");
// 		}
	
		// COMPUTE REPLACEMENT LINE
			// PRINT NAME
			replacement_line = "                ";
			replacement_line = concat (replacement_line, "{\"");
			replacement_line = concat (replacement_line,  name);
			replacement_line = concat (replacement_line, "\",");

			// PRINT INDENTATION
			// compute lenght of name 	
			charcount = 0;
			for(i=0; name[i]; i++) {
				charcount ++;
			}
	
			// substract lenght of name to max indetation
			indent = MAX_INDENT - charcount;
	
			// appy indentation 
			for (i=0; i<indent; i++){
				replacement_line = concat(replacement_line, " ");
			}
		
			// PRINT DATA TO REPLACEMENT LINE
			if (strcmp(format, "float") == 0){
				// FIXME: CAUSES ABORT 6 ERROR
				for (i=0; i<TABLELEN-1; i++){
					sprintf(value_buf, "%f, ",waveform[i]);
					replacement_line = concat (replacement_line, value_buf);
				}
				sprintf(num_buf, "%f}, ",waveform[i]);
				replacement_line = concat (replacement_line, value_buf); 
			
			} else if (strcmp(format, "16bit") == 0){
				for (i=0; i<TABLELEN-1; i++){
					sprintf(value_buf, "%d, ", (int16_t)(waveform[i]* (float)(MAX16BITS)));
					replacement_line = concat (replacement_line, value_buf);
				}
				sprintf(value_buf, "%d}, ", (int16_t)(waveform[TABLELEN-1]* (float)(MAX16BITS)));
				replacement_line = concat (replacement_line, value_buf);

			} else if (strcmp(format, "8bit") == 0){
				for (i=0; i<TABLELEN-1; i++){
					sprintf(value_buf, "%d, ", (int8_t)(waveform[i]* (float)(MAX8BITS)));
					replacement_line = concat (replacement_line, value_buf);
				}
				sprintf(value_buf, "%d}, ", (int8_t)(waveform[TABLELEN-1]* (float)(MAX8BITS)));
				replacement_line = concat (replacement_line, value_buf);
			}
		
			// escape dots for SED to work
			replace(replacement_line, '.', "\\.");
	
		// temporary file
		// system("cp ../libraries/wavetable/float/wavetable_struct.c ../libraries/wavetable/float/wavetable_struct_tmp.c");
		temp_filename = concat (filename, ".tmp");
		command_copy = concat ("cp ", filename);
		command_copy = concat (command_copy, " ");
		command_copy = concat (command_copy, temp_filename);
		system(command_copy);

		// update sections		
		// command = "sed '/SAW/s/.*/SAW_BLAH/' ../libraries/wavetable/float/wavetable_struct_tmp.c > ../libraries/wavetable/float/wavetable_struct.c";
		command = "sed '/";
		command = concat(command, name);
		command = concat(command, "/s/.*/");
		command = concat(command, replacement_line);
		command = concat(command, "/' ");
		command = concat(command, temp_filename);
		command = concat(command, " > ");
		command = concat(command, filename);
		system(command);
	
		// remove temporary file
		system(concat("rm ", temp_filename));


		// CLOSE WAVETABLE FILE
		// 	fclose(fp);	
		// 	free(filename);
	}
}

char* concat(const char *s1, const char *s2)
{
	char *result = malloc(strlen(s1)+strlen(s2)+1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

char* replace(const char *s, char ch, const char *repl) 
// function comes from:
// http://stackoverflow.com/questions/12890008/replacing-character-in-a-string
{
	int count = 0;
	const char *t;
	for(t=s; *t; t++)
		count += (*t == ch);

	size_t rlen = strlen(repl);
	char *res = malloc(strlen(s) + (rlen-1)*count + 1);
	char *ptr = res;
	for(t=s; *t; t++) {
		if(*t == ch) {
			memcpy(ptr, repl, rlen);
			ptr += rlen;
		} else {
			*ptr++ = *t;
		}
	}
	*ptr = 0;
	return res;
} 


void plot_wavetable(char plot_title[1024], float waveform[TABLELEN], int tablelen, int color)
{

	// PLOT WAVETABLES
	// code inspired by following stackoverflow thread:
	// http://stackoverflow.com/questions/3521209/making-c-code-plot-a-graph-automatically
	
	char *gnuplot_Settings = "gnuplot -p -e \" "
							 "set terminal x11 size 1200,750 enhanced font 'Verdana,10' persist;"
							 "set border linewidth 1.5;"
							 "set xrange [0:512];"							 
// 							 "set yrange [-1:1];"
							 "set style line 1 linecolor rgb '#DC143C' linetype 1 linewidth 2;"
							 "set style line 2 linecolor rgb '#228B22' linetype 1 linewidth 2;"
							 "set style line 3 linecolor rgb '#0000FF' linetype 1 linewidth 2;";

	int i;
	int ret;							 				
	char gnuplot_Commands[800];
	strcpy(gnuplot_Commands, gnuplot_Settings);
	strcat(gnuplot_Commands, "set title \'");
	strcat(gnuplot_Commands, plot_title);
	strcat(gnuplot_Commands, "\' ; ");		
	if (color == 1){strcat(gnuplot_Commands, "plot \'data.temp\' with lines linestyle 1;\"");}
	if (color == 2){strcat(gnuplot_Commands, "plot \'data.temp\' with lines linestyle 2;\"");}
	if (color == 3){strcat(gnuplot_Commands, "plot \'data.temp\' with lines linestyle 3;\"");}
	
	// Write data to temporary file
 	FILE * temp 		= fopen("data.temp", "w");
	FILE * gnuplotPipe 	= popen ("gnuplot -persistent", "w");
	for (i=0; i < tablelen; i++){
		fprintf(temp, "%d %lf \n", i, waveform[i]); 
	}

	// Plot from temporary file
	// printf("- Plot wavetable\n");
	system(gnuplot_Commands);

	// Close  temporary file	
	fclose(temp);

	// Delete temporaty file:
	ret = remove("data.temp");
	// if(ret == 0) {printf("- Delete temporary gnuplot file\n");}
	// else{printf("X - Error: unable to delete the file");}
	
}

// Exploit the Central Limit Theorem (``law of large numbers'') and add up several uniformly-distributed random numbers
// source: http://c-faq.com/lib/gaussian.html
#define NSUM 		25
float gaussrand()
{
	float x = 0;
	int i;
	for(i = 0; i < NSUM; i++)
		x += (float)rand() / RAND_MAX;

	x -= NSUM / 2.0;
	x /= sqrt(NSUM / 12.0);

	return x;
}

