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


#ifndef M_PI
#define M_PI           3.14159265358979323846 
#endif

// GLOBAL
#define FILE_TABLELEN 		512 	// FixMe: This must be set to match the wav files read, but ideally should be detected in the wav file
#define	WT_DIM_SIZE	3
#define OSC_OUTPUT_TABLELEN 512
#define LFO_OUTPUT_TABLELEN 256


const char default_table_author[] 	= "Dan Green";
const char default_input_dir[] 		= "../libraries/wavefiles/";

const char default_spherename[] 	= "hp_lfo_ramp";
const char default_output_dir[] 	= "../libraries/spheres/";
const char default_osc_lfo[] 		= "OSC";


char do_smoothing 			= 0;
char do_remove_DC_offset 	= 0;
char do_cosine_window 		= 0;
char do_normalize 			= 0;
	
char DEBUG_PRINT_ALL_SAMPLES = 0;


uint16_t 	OUTPUT_TABLELEN;
float 		bitdepth_adj;
float 		bitdepth_gain;


// WAVE-FILES
#define HEADERLEN 312
#define NUM_WAV_WAVEFORMS 200 		// keep this number larger than the actual number of waveforms


// PLOTTING
#define NUM_COMMANDS 2
char plot_waveform[300];




// WAVE-FILE HEADER FORMAT
struct HEADER {
    unsigned char riff[4];                      // RIFF string
    unsigned int overall_size   ;               // overall size of file in bytes
    unsigned char wave[4];                      // WAVE string
    unsigned char fmt_chunk_marker[4];          // fmt string with trailing null char
    unsigned int length_of_fmt;                 // length of the format data
    unsigned int format_type;                   // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    unsigned int channels;                      // no.of channels
    unsigned int sample_rate;                   // sampling rate (blocks per second)
    unsigned int byterate;                      // SampleRate * NumChannels * BitsPerSample/8
    unsigned int block_align;                   // NumChannels * BitsPerSample/8
    unsigned int bits_per_sample;               // bits per sample, 8- 8bits, 16- 16 bits etc
    unsigned char data_chunk_header [4];        // DATA string or FLLR string
    unsigned int data_size;                     // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};
struct HEADER header[200];
unsigned char buffer4[4];
unsigned char buffer2[2];


// WAVETABLE STRUCTURE
struct wavetable{
	 int16_t data[FILE_TABLELEN];
	 int coordinates[4];
	 char name[200];			// extend if names get truncated
	 int num_waveforms; 		// X, Y and Z.
	 int max_dimension;
	 int num_banks;
};

struct wavetable wavetable[NUM_WAV_WAVEFORMS];

// FUNCTIONS
void phasematch(float output[FILE_TABLELEN+1], float *buff[FILE_TABLELEN+1], int desired_first_sample);
// char* concat(const char *s1, const char *s2);
uint8_t add_slash(char *string);
uint8_t trim_slash(char *string);

void plot_wavetable(char plot_title[1024],int dof, float waveform[NUM_WAV_WAVEFORMS]);

void print_wavetable_to_file(char *filename, char *input_wav_dir, char *spherename, char *table_author, char *osc_lfo, struct wavetable wavetable[NUM_WAV_WAVEFORMS]);

void print_usage(void);

void print_usage(void)
{
	printf ("\
#########################	\n\
\twavecalc			\n\
#########################	\n\
Usage: \n\
wavecalc [input_wav_dir [output_dir [author[osc_lfo]]]]\n\
./wavecalc \"../libraries/2018_08_05_HP_BetaTest_LFO_BANKS/\" \"../libraries/spheres/\" \"Hugo Paris hugoplho@gmail.com\" \"OSC\" \n\
./wavecalc \"../libraries/2018_08_05_HP_BetaTest_LFO_BANKS/\" \"../libraries/spheres/\" \"Hugo Paris hugoplho@gmail.com\" \"LFO\" \n\
\n\
The spherename will be extracted from input_wav_dir.\n\
The output will be put into output_dir/spherename.h\n\
\n\
\n\
Default values:\n\
--------------\n\
input_wav_dir: %s%s\n\
output_dir: %s\n\
author: %s\n\
oscillator / lfo: %s\n\
\n\
\n\
After running wavecalc:\n\
----------------------\n\
1) Copy the .h file into the SWN folder inc/spheres/\n\
2) Add the following line to the top of SWN_PROJECT_DIR/inc/spheres_internal.h:\n\
\t#include \"spheres/spherename.h\"\n\
3) Add (void *)spherename inside the declaration for const void *wavetable_list[]={...} in spheres_internal.h\n\
\n\
\n\
"\
	, default_input_dir, default_spherename, default_output_dir, default_table_author, default_osc_lfo);
}

int main(int argc, char *argv[])
{
	int i;
	uint32_t j;
	int 		k 		= 0;
	int 		l 		= 0;
	int 		dof 	= 0; 			// degree of freedom: x,y,z
	int 		read 	= 0;
	float 		inc;	
	float 		sum_buf;
	float 		temp[FILE_TABLELEN];	
	float 		output[FILE_TABLELEN];
	float* 		buff[FILE_TABLELEN];
	uint32_t 	header_buf[HEADERLEN];
	uint32_t 	maxval_buf;
	char 		filename[256];
	FILE 		* ptr; 					//wav file pointer
	DIR 		*d;
	struct 		dirent 	*dir;
	char 		*input_wav_dir;
	char 		*output_filename;
	char 		*spherename;
	char 		*table_author;
	char		*test_str;
	char 		*osc_lfo;


	input_wav_dir = malloc(255);
	output_filename = malloc(255);
	spherename = malloc(255);
	table_author = malloc(255);
	osc_lfo = malloc(255);

	if (argc==1) {print_usage();}

	//Extract input dir from args, or use default
	if (argc>1 && argv[1][0]) {

		if (strcmp(argv[1], "help") == 0) {print_usage(); return 1;}

		strcpy(input_wav_dir, argv[1]);
		trim_slash(input_wav_dir);

		test_str = strrchr(input_wav_dir, '/');
		if (!test_str) {
			strcpy(spherename, argv[1]);
			strcpy(input_wav_dir, "./");
		} else {
			strcpy(spherename, test_str);
			spherename++;
			trim_slash(spherename);
			add_slash(input_wav_dir);
		}
	}
	else {
		strcpy(spherename, default_spherename);
		strcpy(input_wav_dir, default_input_dir);
		add_slash(input_wav_dir);
		strcat(input_wav_dir, spherename);
		add_slash(input_wav_dir);
	}	

	// Extract output dir from args, or use default
	if (argc>2 && argv[2][0]) { 
		strcpy(output_filename, argv[2]);
	}
	else {
		strcpy(output_filename, default_output_dir);
	}

	if (argc>3 && argv[3][0]) {
		strcpy(table_author, argv[3]);
	} else {
		strcpy(table_author, default_table_author);
	}

	if (argc>4 && argv[4][0]) {
		strcpy(osc_lfo, argv[4]);
	} else {
		strcpy(osc_lfo, default_osc_lfo);
	}


	add_slash(output_filename);
	if (strcmp (osc_lfo, "LFO") == 0){strcat(output_filename, "LFO_");}
	strcat(output_filename, spherename);
	strcat(output_filename, ".h");


	// OSCILLATOR // LFO
	if 		(strcmp (osc_lfo, "OSC") == 0){OUTPUT_TABLELEN = OSC_OUTPUT_TABLELEN; bitdepth_adj = 0.0; 	  bitdepth_gain = 1.0;}	
	else if (strcmp (osc_lfo, "LFO") == 0){OUTPUT_TABLELEN = LFO_OUTPUT_TABLELEN; bitdepth_adj = 126.0;   bitdepth_gain = 255.0 / 65535.0;}


	printf("\n\n\
Input wave dir: %s\n\
Sphere name: %s\n\
Output file: %s\n\
Author: %s\n\
oscillator / lfo: %s\n", input_wav_dir, spherename, output_filename, table_author, osc_lfo);


	// X-loop: filling up one row
	// Y-loop: then filling up another row on Y+=1 until columns are complete
	// Z-loop: move on to next layer (z+=1) until all layers are complete
	int cx=0;
	int cy=0;
	int cz=0;
	int cbank=0;


	// FIXME: ADD BACKUP BEFORE MOVING
		
	// REMOVE PREVIOUS WAVEFORM FILES FROM FOLDER
	//system("exec rm -r ../waveforms/*.h");

	// MAKE COSINE WINDOW
	float cosine_window[FILE_TABLELEN];
	for (i=0; i<FILE_TABLELEN; i++){
		cosine_window[i] = sin(i*M_PI/(FILE_TABLELEN-1)); 
		cosine_window[i] *= cosine_window[i]; 
	}
	
	//WIDEN COSINE WINDOW
	int cosine_gain = 100;
	for (i=0; i<FILE_TABLELEN; i++){
		cosine_window[i] *= cosine_gain;
		if (cosine_window[i] > 1){cosine_window[i]=1;}
	}

/*
	// PLOT WINDOW
						
		//Write data to temporary file
		FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
		for (i=0; i < FILE_TABLELEN; i++){
			fprintf(tempb, "%d %lf \n", i, cosine_window[i]); 
		}
	
		//Send commands to gnuplot one by one.
		for (i=0; i < NUM_COMMANDS; i++){
			fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); 
		}
*/


	// LARGEST WAVETABLE DIMENSION AVAILABLE
	// OPEN FOLDER
	d = opendir(input_wav_dir);
	if (d){
	
		// SCAN WAV-FILES IN FOLDER
		while ((dir = readdir(d)) != NULL){
		
			// SKIP OSX FILES 
			// osx-related files and non-wav
			char * end = strrchr(dir->d_name, '.');
			if ( (strcmp(dir->d_name,".") !=0 ) && (strcmp(dir->d_name,"..") !=0) && (strcmp(dir->d_name,".DS_Store") !=0)  && (strcmp(end, ".wav") == 0) ){
				l+=1;
			}
		}
	} else 
	{
		printf("\n\nError: input_wav_dir cannot be opened\n\n");
		exit(0);
	}

	wavetable[0].num_waveforms 		= l;
	// wavetable[0].max_dimension	= (int)(cbrt(wavetable[0].num_waveforms));

	wavetable[0].max_dimension	= WT_DIM_SIZE;
	wavetable[0].num_banks	= (int)(l / (wavetable[0].max_dimension * wavetable[0].max_dimension * wavetable[0].max_dimension));

	l=0;
	printf("\n\n#####################################################################");
	printf("\n input: %s ", input_wav_dir);
	printf("\n output: %s \n", output_filename);
	printf("\n %u WAV files available", wavetable[0].num_waveforms);
	printf("\n Maximum wavetable dimensions: %u x %u x %u", wavetable[0].max_dimension, wavetable[0].max_dimension, wavetable[0].max_dimension);
	printf("\n Number of banks: %u \n", wavetable[0].num_banks);
	printf("#####################################################################\n\n");
	closedir(d);
	
			
	// OPEN FOLDER
	d = opendir(input_wav_dir);
	if (d){
	
		// SCAN WAV-FILES IN FOLDER
		while ((dir = readdir(d)) != NULL){
		
			// SKIP OSX FILES 
			// osx-related files and non-wav
			char * end = strrchr(dir->d_name, '.');
			if ( (strcmp(dir->d_name,".") !=0 ) && (strcmp(dir->d_name,"..") !=0) && (strcmp(dir->d_name,".DS_Store") !=0)  && (strcmp(end, ".wav") == 0) ){
			
				// ASSIGN COORDINATES
				wavetable[l].coordinates[0]=cx;
				wavetable[l].coordinates[1]=cy;
				wavetable[l].coordinates[2]=cz;
				wavetable[l].coordinates[3]=cbank;
				printf("--------------------------------");
				printf("(%d: %d%d%d)",wavetable[l].coordinates[3],wavetable[l].coordinates[0],wavetable[l].coordinates[1],wavetable[l].coordinates[2]);
				printf("--------------------------------\n");		

				// SAVE FILE NAME AS WAVEFORM NAME
				strcpy(wavetable[l].name, dir->d_name);
				printf("FILE: \n%s\n\n", dir->d_name);

				//LOAD CONTENT FROM WAV
				//open wav files
				sprintf(filename, "%s/%s", input_wav_dir, dir->d_name);
				ptr = fopen(filename, "r");
				if (ptr == NULL) {
					printf("Error opening file\n");
				}

				// READ HEADER
				// 44 bytes <=> 352 bits)	
				// code from: http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
				 printf("HEADER: \n");
				 read = fread(header[k].riff, sizeof(header[k].riff), 1, ptr);
				 printf("(1-4): %s \n", header[k].riff); 

				 read = fread(buffer4, sizeof(buffer4), 1, ptr);
				 printf("%#x %#x %#x %#x\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

				 // convert little endian to big endian 4 byte int
				 header[k].overall_size  = buffer4[0] | 
										  (buffer4[1]<<8) | 
										  (buffer4[2]<<16) | 
									  	  (buffer4[3]<<24);

				 printf("(5-8) Overall size: bytes:%u, Kb:%u \n", header[k].overall_size, header[k].overall_size/1024);

				 read = fread(header[k].wave, sizeof(header[k].wave), 1, ptr);
				 printf("(9-12) Wave marker: %s\n", header[k].wave);

				 read = fread(header[k].fmt_chunk_marker, sizeof(header[k].fmt_chunk_marker), 1, ptr);
				 printf("(13-16) Fmt marker: %s\n", header[k].fmt_chunk_marker);

				 read = fread(buffer4, sizeof(buffer4), 1, ptr);
				 printf("%#x %#x %#x %#x\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

				 // convert little endian to big endian 4 byte integer
				 header[k].length_of_fmt =   buffer4[0] |
											(buffer4[1] << 8) |
											(buffer4[2] << 16) |
											(buffer4[3] << 24);
				 printf("(17-20) Length of Fmt header: %u \n", header[k].length_of_fmt);

				 read = fread(buffer2, sizeof(buffer2), 1, ptr); printf("%#x %#x \n", buffer2[0], buffer2[1]);

				 header[k].format_type = buffer2[0] | (buffer2[1] << 8);
				 char format_name[10] = "";
				 if (header[k].format_type == 1)
				   strcpy(format_name,"PCM"); 
				 else if (header[k].format_type == 6)
				  strcpy(format_name, "A-law");
				 else if (header[k].format_type == 7)
				  strcpy(format_name, "Mu-law");

				 printf("(21-22) Format type: %u %s \n", header[k].format_type, format_name);

				 read = fread(buffer2, sizeof(buffer2), 1, ptr);
				 printf("%#x %#x \n", buffer2[0], buffer2[1]);

				 header[k].channels = buffer2[0] | (buffer2[1] << 8);
				 printf("(23-24) Channels: %u \n", header[k].channels);

				 read = fread(buffer4, sizeof(buffer4), 1, ptr);
				 printf("%#x %#x %#x %#x\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

				 header[k].sample_rate = buffer4[0] |
										(buffer4[1] << 8) |
										(buffer4[2] << 16) |
										(buffer4[3] << 24);

				 printf("(25-28) Sample rate: %u\n", header[k].sample_rate);

				 read = fread(buffer4, sizeof(buffer4), 1, ptr);
				 printf("%#x %#x %#x %#x\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

				 header[k].byterate  = buffer4[0] |
									  (buffer4[1] << 8) |
									  (buffer4[2] << 16) |
									  (buffer4[3] << 24);
				 printf("(29-32) Byte Rate: %u , Bit Rate:%u\n", header[k].byterate, header[k].byterate*8);

				 read = fread(buffer2, sizeof(buffer2), 1, ptr);
				 printf("%u %u \n", buffer2[0], buffer2[1]);

				 header[k].block_align = buffer2[0] |
									(buffer2[1] << 8);
				 printf("(33-34) Block Alignment: %u \n", header[k].block_align);

				 read = fread(buffer2, sizeof(buffer2), 1, ptr);
				 printf("%u %u \n", buffer2[0], buffer2[1]);

				 header[k].bits_per_sample = buffer2[0] |
									(buffer2[1] << 8);
				 printf("(35-36) Bits per sample: %u \n", header[k].bits_per_sample);

				 read = fread(header[k].data_chunk_header, sizeof(header[k].data_chunk_header), 1, ptr);
				 printf("(37-40) Data Marker: %s \n", header[k].data_chunk_header);

				 read = fread(buffer4, sizeof(buffer4), 1, ptr);
				 printf("%#x %#x %#x %#x\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

				 header[k].data_size = buffer4[0] |
									  (buffer4[1] << 8) |
									  (buffer4[2] << 16) | 
									  (buffer4[3] << 24 );
				 printf("(41-44) Size of data chunk: %u \n", header[k].data_size);
			
				 // calculate no.of samples
				 long num_samples = (8 * header[k].data_size) / (header[k].channels * header[k].bits_per_sample);
				 printf("Number of samples:%lu \n", num_samples);

				 long size_of_each_sample = (header[k].channels * header[k].bits_per_sample) / 8;
				printf("Size of each sample:%ld bytes\n\n", size_of_each_sample);
			
				// READ ONE WAVETABLE LENGHT WORTH OF WAVEFILE
				// read each sample from data chunk if PCM
				printf("DATA:");
				if (header[k].format_type == 1) { // PCM
					long i =0;
					int a=0;
					int b=0;
					int size_is_correct = 1;

					// make sure that the bytes-per-sample is completely divisible by num.of channels
					long bytes_in_each_channel = (size_of_each_sample / header[k].channels);
					if ((bytes_in_each_channel  * header[k].channels) != size_of_each_sample) {
						printf("Error: %ld x %ud <> %ld\n", bytes_in_each_channel, header[k].channels, size_of_each_sample);
						size_is_correct = 0;
					}
					else printf("bytes_in_each_channel = %ld\n", bytes_in_each_channel);

					// unsigned char data_buffer[bytes_in_each_channel];
					unsigned char data_buffer[24];

					if (size_is_correct) { 
						// the valid amplitude range for values based on the bits per sample
						long low_limit = 0l;
						long high_limit = 0l;

						switch (header[k].bits_per_sample) {
							case 8:
								low_limit = -128;
								high_limit = 127;
								break;
							case 16:
								low_limit = -32768;
								high_limit = 32767;	//0x7FFF
								break;
							case 24:
								low_limit = -8388608;
								high_limit = 8388607; //0x7FFFFF
							break;
							case 32:
								low_limit = -2147483648;
								high_limit = 2147483647; //0x7FFFFFFF
								break;
							}
						printf("\nValid range for data values : %ld to %ld \n", low_limit, high_limit);
					
						// Skip 10x table lenght worth of samples 
						// skips attack and potential blanks
						// for (i =0; i < 10*FILE_TABLELEN; i++) {
						// 	read = fread(data_buffer, sizeof(data_buffer), 1, ptr);
						// }
					
						// read one table length worh of samples
						// FIXME: repeat this section a couple times

						unsigned int  xchannels = 0;
						int data_in_channel = 0;
						uint8_t data_in_channel_8=0;

						for (i =0; i < FILE_TABLELEN; i++) {

							for (xchannels = 0; xchannels < header[k].channels; xchannels ++ ) {
								read = fread(data_buffer, bytes_in_each_channel, 1, ptr);
 								// printf("channel %d of %d\n",xchannels, header[k].channels);

								if (read == 1){
	
									// dump the data read for 1 channel
									if (xchannels == 0){

										// convert data from little endian to big endian based on bytes in each channel sample
										// converts to signed integer with sample file's native bit-depth

										if (bytes_in_each_channel == 4) {
											data_in_channel =	(int32_t)
																(data_buffer[0] 		| 
																(data_buffer[1] << 8) 	| 
																(data_buffer[2] << 16) 	| 
																(data_buffer[3] << 24) );

											if (DEBUG_PRINT_ALL_SAMPLES) printf("%02x%02x%02x%02x = %d\n", data_buffer[3], data_buffer[2], data_buffer[1], data_buffer[0], data_in_channel);
										}
										else if (bytes_in_each_channel == 3) {
											data_in_channel = (int32_t)
																((data_buffer[0] <<8)	|
																(data_buffer[1] << 16) 	| 
																(data_buffer[2] << 24) );

											data_in_channel = data_in_channel/(1<<8); //32 to 24 bit conversion

											if (DEBUG_PRINT_ALL_SAMPLES) printf("%02x%02x%02x = %d\n", data_buffer[2], data_buffer[1], data_buffer[0], data_in_channel);
										}
										else if (bytes_in_each_channel == 2) {
											data_in_channel = (int16_t)
																(data_buffer[0] 		| 
																(data_buffer[1] << 8) );

											if (DEBUG_PRINT_ALL_SAMPLES) printf("%02x%02x = %d\n", data_buffer[1], data_buffer[0], data_in_channel);
										}
										else if (bytes_in_each_channel == 1) {
											data_in_channel_8 = (uint8_t) data_buffer[0];
											data_in_channel = data_in_channel_8 - 128;
											if (DEBUG_PRINT_ALL_SAMPLES) printf("%02x = %d = %d\n", data_buffer[0], data_in_channel_8, data_in_channel);
										}
								
										if (data_in_channel < low_limit || data_in_channel > high_limit){
											if (DEBUG_PRINT_ALL_SAMPLES) printf("**value out of range\n");
										}

										// convert signed integer in sample file's native bit depth to our wavetable bit-depth
										// We're using 16-bit signed ints for now:
										//
										if (bytes_in_each_channel == 4) {
											wavetable[l].data[b]=(int16_t)(data_in_channel/32768);
										} else if (bytes_in_each_channel == 3) {
											wavetable[l].data[b]=(int16_t)(data_in_channel/256);
										} else if (bytes_in_each_channel == 2) {
											wavetable[l].data[b]=(int16_t)(data_in_channel);
										} else if (bytes_in_each_channel == 1) {
											wavetable[l].data[b]=(int16_t)(data_in_channel*256);
										}

										//float:
										// wavetable[l].data[b]=data_in_channel;

										// apply OSC/LFO mode gain
										wavetable[l].data[b] = (uint16_t)((float)(wavetable[l].data[b]) * bitdepth_gain + bitdepth_adj) ;
										if 		((wavetable[l].data[b]<0)  	 && (strcmp (osc_lfo, "LFO") == 0)){wavetable[l].data[b] = 0;}
										else if ((wavetable[l].data[b]>255)  && (strcmp (osc_lfo, "LFO") == 0)){wavetable[l].data[b] = 255;}

										b+=1;

									}

								}else {
									printf("Error reading file. %d bytes %ld sizeof(data_buffer)\n", read, sizeof(data_buffer));
									break;
								}
							}
						} 
						printf("- Stereo -> Mono\n");

					} 

				}
				else{
					printf("ERROR: Not PCM \n");
				}	
			


//					plot_wavetable("RAW",wavetable[l].data);

				if (do_smoothing){
					// SMOOTHING:
					// LINEAR INTERPOLATION FOR NEAR-ZERO SAMPLES
					for (j=0; j<10; j++){
						for (i=0; i<FILE_TABLELEN; i++){
							if (((abs(wavetable[l].data[i]))<1000)&&
								(i!=0) && (i<FILE_TABLELEN-1)){
								wavetable[l].data[i] = (wavetable[l].data[i-1] + wavetable[l].data[i+1])/2;
							}
						}	
					}
// 						plot_wavetable("SMOOTH",wavetable[l].data);
				}
							
				if (do_remove_DC_offset){			
					// DETECT DC OFFSET
					sum_buf=0;
					for (i=0; i<FILE_TABLELEN; i++){
						sum_buf += wavetable[l].data[i];
					}
				}

				if (do_cosine_window){
					// APPLY WINDOW
					for (i=0; i<FILE_TABLELEN; i++){
						wavetable[l].data[i] *= cosine_window[i];
					}
// 						plot_wavetable("WINDOW",wavetable[l].data);
				}

				if (do_remove_DC_offset){			
					// REMOVE DC OFFSET
					for (i=0; i<FILE_TABLELEN; i++){
						wavetable[l].data[i] -= (sum_buf/FILE_TABLELEN);
					}
// 						plot_wavetable("OFFSET REMOVED",wavetable[l].data);
				}

				// FIXME: APPLY LPF
				// maybe this needs to be a band pass

				if (do_normalize){					
					// FIND MAX AMPLITUDE
					maxval_buf =0;
					for (i=0; i<FILE_TABLELEN; i++){
						if (abs(wavetable[l].data[i]) > maxval_buf){
							maxval_buf = abs(wavetable[l].data[i]);
						} 
					}				

					// NORMALIZE WAVEFORM GAIN 
					// save to wavetable
					for (i=0; i<FILE_TABLELEN; i++){
						wavetable[l].data[i] /= maxval_buf;
					}	
					printf("- Normalization -> saved to wavetable\n");
//	 					plot_wavetable("NORMALIZED",wavetable[l].data);
				}
				

				// ASSIGN COORDINATES
				
				// PLOT WAVETABLES
				//plot_wavetable(dir->d_name,dof,wavetable[l].data);

				// SAVE WAVEFORM TO INDIVIDUAL FILE
// 				print_waveform_to_file(wavetable[k]);
			
				printf("\nClosing file..\n");
				fclose(ptr);
				
				
				// UPDATE COORDINATES
				cx  += 1;
				if (cx == wavetable[0].max_dimension){
					cx = 0;
					cy += 1;
					if (cy == wavetable[0].max_dimension){
						cy  = 0;
						cx  = 0;
						cz += 1;
						if (cz == wavetable[0].max_dimension){
							cz  = 0;
							cbank += 1;
						}
					}
				}
				
				//Advance to next wavetable[] element
				l += 1;

				maxval_buf=0;
		
			}
		}
		closedir(d);
	}
	printf("\n\n\nCreating file: %s\n\n", output_filename);

	// SAVE WAVETABLE TO FILE
	print_wavetable_to_file(output_filename, input_wav_dir, spherename, table_author, osc_lfo, wavetable);


}
	
	





//---------- FUNCTIONS ---------

void print_wavetable_to_file(char *filename, char *input_wav_dir, char *spherename, char *table_author, char *osc_lfo, struct wavetable wavetable[NUM_WAV_WAVEFORMS])
{
	char *table_path;
	uint32_t i,layer,row,element, bank;
	int l,m;

	table_path = malloc(strlen(spherename)+256);//max filepath length

	freopen(filename, "w", stdout);

	
	if(strcmp(osc_lfo,"OSC")==0){
		
		printf("// This file generated with wavecalc\n");
		printf("// Authors: Hugo Paris, hugoplho@gmail.com, Dan Green danngreen1@gmail.com \n//\n");
		printf("// 1) Place this file into SWN_PROJECT_DIR/inc/spheres/\n");
		printf("// 2) Add the following line to the top of SWN_PROJECT_DIR/inc/spheres_internal.h:\n");
		printf("//	#include \"spheres/%s.h\"\n", spherename);
		printf("// 3) Add (void *)%s inside the declaration for const void *wavetable_list[]={...} in spheres_internal.h\n", spherename);
		printf("// -------------------------------------------------------\n//\n");
		printf("// Wavetable name: %s\n", spherename);
		printf("// Wavetable by: %s\n", table_author);
		printf("// Wavefiles for waveforms source directory: %s\n", input_wav_dir);
		printf("// oscillator / lfo: %s\n\n", osc_lfo);

		for (bank=0; bank<wavetable[0].num_banks; bank++){

			printf("\nconst o_waveform %s[WT_DIM_SIZE][WT_DIM_SIZE][WT_DIM_SIZE] = \n\n", spherename);  
		
			i=0;
			printf("{\n");

			for (layer=0; layer<wavetable[0].max_dimension; layer++){
				printf("\t// ##################\n");
				printf("\t//     LAYER %d\n", wavetable[i].coordinates[2]+1 );
				printf("\t// ##################\n\n");
				printf("\t{\n");

				for (row=0; row<wavetable[0].max_dimension; row++){
					printf("\t\t// ROW %d\n", wavetable[i].coordinates[1]+1 );
					printf("\t\t{\n");

					for (element=0; element<wavetable[0].max_dimension; element++){
						strcpy(table_path, spherename);
						strcat(table_path, "/");
						strcat(table_path, wavetable[i].name);
						printf("\t\t\t{{\"%s\"},{\t\t\t%d", table_path, wavetable[i].data[0]);
						for (l=1; l<OUTPUT_TABLELEN; l++){
							printf(",%d", wavetable[i].data[l]);
						}

						//Double the table if we only read 256 (we need 512 for SWN code)
						if ((FILE_TABLELEN*2) == OUTPUT_TABLELEN){
							for (l=0; l<FILE_TABLELEN; l++){	
								printf(",%d", wavetable[i].data[l]);
							}
						}

						i+=1;

						if (element<(wavetable[0].max_dimension-1)) {printf("}},\n");}
						else {printf("}}\n");}
					}
					if (row<(wavetable[0].max_dimension-1)) {printf("\t\t},\n");}
					else {printf("\t\t}\n");}
				}
				if (layer<(wavetable[0].max_dimension-1)) {printf("\t},\n\n\n");}
				else {printf("\t}\n");}
			}
			if (bank<(wavetable[0].num_banks-1)) {printf("},\n\n\n\n");}
			else {printf("};\n\n\n");}
		}
	}


	else if(strcmp(osc_lfo,"LFO")==0){

		printf("// This file generated with wavecalc\n");
		printf("// Authors: Hugo Paris, hugoplho@gmail.com, Dan Green danngreen1@gmail.com \n//\n");
		
		printf("\n\n// 1) SLPLIT LFO_WAVETABLE[][] ARRAY INTO BANKS BY UPDATING LFO_TO_BANK_END ARRAY BELOW\n");
		printf("// 2) Comment out enused waveforms and remove coma at end of last waveform \n");
		printf("// 3 Update value for NUM_LFO_GROUPS in lfo_wavetable_bank.h\n");
		printf("// 4) Copy the content of this file into src/lfo_wavetable_bank.c/\n");

		printf("// -------------------------------------------------------\n//\n");
		printf("// Wavetable name: %s\n", spherename);
		printf("// Wavetable by: %s\n", table_author);
		printf("// Wavefiles for waveforms source directory: %s\n", input_wav_dir);
		printf("// oscillator / lfo: %s\n\n", osc_lfo);


		printf("#include \"arm_math.h\" \n#include \"lfo_wavetable_bank.h\"\n");
		printf("\nuint8_t LFOS_TO_BANK_END[NUM_LFO_GROUPS] = {4, 10, 16, 22, NUM_LFO_SHAPES};\n");

		
		for (bank=0; bank<wavetable[0].num_banks; bank++){

			printf("\nconst uint8_t lfo_wavetable[NUM_LFO_SHAPES][LFO_TABLELEN] = \n{ \n");
			i=0;

			for (layer=0; layer<wavetable[0].max_dimension; layer++){

				for (row=0; row<wavetable[0].max_dimension; row++){
					
					m=2;
					for (element=0; element<wavetable[0].max_dimension; element++){
						
						printf("\t{%d", 0);
						for (l=1; l<FILE_TABLELEN-1; l++){
							if(m==2){printf(",%d", wavetable[i].data[l]); m=0;}
							m++;
						}

						i+=1;

						printf("},\n");
					}
				}
			}
			if (bank<(wavetable[0].num_banks-1)) {printf("},\n\n\n\n");}
			else {printf("};\n\n\n");}
		}
	}


	fclose(stdout);

	free(table_path);
}



//adds trailing slash if it doesn't already exist
//returns 1 if slash was added, 0 if it already existed
uint8_t add_slash(char *string)
{
  uint8_t len;

  len = strlen(string);

  if (string[len-1]=='/') 
    return (0); //not added
  else {
    string[len]   = '/';
    string[len+1] = '\0';
    return(1); //added
  }
}

//removes trailing slash if it exists
//returns 1 if slash existed, 0 if not
uint8_t trim_slash(char *string)
{
  uint8_t len;

  len = strlen(string);

  if (string[len-1]=='/') {
    string[len-1] = 0;
    return(1); //trimmed
  }
  else
    return(0); //not trimmed
}






//---------- BACKUP ---------



void plot_wavetable(char plot_title[1024], int dof, float waveform[NUM_WAV_WAVEFORMS]){

	// PLOT WAVETABLES
	// code inspired by following stackoverflow thread:
	// http://stackoverflow.com/questions/3521209/making-c-code-plot-a-graph-automatically
	
// 	char tablelen[15];
// 	sprintf(tablelen, "%d", TABLELEN);

	char *gnuplot_Settings = "gnuplot -p -e \" "
							 "set terminal x11 size 1200,750 enhanced font 'Verdana,10' persist;"
							 "set border linewidth 1.5;"
// 							 "set yrange [0:512];"							 
// 							 "set yrange [-1:1];"
							 "set style line 1 linecolor rgb '#DC143C' linetype 1 linewidth 2;"
							 "set style line 2 linecolor rgb '#228B22' linetype 1 linewidth 2;"
							 "set style line 3 linecolor rgb '#0000FF' linetype 1 linewidth 2;";

// 	strcat(gnuplot_Settings,"set xrange [0:");
// 	strcat(gnuplot_Settings, tablelen);
// 	strcat(gnuplot_Settings,"];");
	int i;
	int ret;							 				
	char gnuplot_Commands[800];
	strcpy(gnuplot_Commands, gnuplot_Settings);
	strcat(gnuplot_Commands, "set title \'");
	strcat(gnuplot_Commands, plot_title);
	strcat(gnuplot_Commands, "\' ; ");		
	if (dof==0){
 		strcat(gnuplot_Commands, "plot \'data.temp\' with lines linestyle 1;\"");
	}
	else if (dof==1){
 		strcat(gnuplot_Commands, "plot \'data.temp\' with lines linestyle 2;\"");
	}
	else if (dof==2){
 		strcat(gnuplot_Commands, "plot \'data.temp\' with lines linestyle 3;\"");
	}
	
	// Write data to temporary file
 	FILE * temp = fopen("tmp/data.temp", "w");
	FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
	for (i=0; i < FILE_TABLELEN; i++){
		fprintf(temp, "%d %lf \n", i, waveform[i]); 
	}

	// Plot from temporary file
	printf("- Plot wavetable\n");
	system(gnuplot_Commands);

	// Close  temporary file	
	fclose(temp);

	// Delete temporaty file:
	ret = remove("tmp/data.temp");
	if(ret == 0) {printf("- Delete temporary gnuplot file\n");}
	else{printf("X - Error: unable to delete the file");}
	
}

void print_waveform_to_file(char* filename, struct wavetable wavetable)
{
	uint32_t i;
	// char* filename = concat(output_dir , wavetable.name);
	// filename = concat(filename, ".h");

 	freopen(filename, "w", stdout);
	printf("%s\n","// this file was made with wavecalc/main.c");
	printf("// Author: Hugo Paris, hugoplho@gmail.com\n\n");
	printf("float %s[%d] = { \n",wavetable.name, FILE_TABLELEN);
	for (i=0; i<FILE_TABLELEN-1; i++){
		printf("%d,\n",wavetable.data[i]);
	}
	printf("%d};\n",wavetable.data[FILE_TABLELEN-1]);	
	fclose(stdout);	
	free(filename);
}

