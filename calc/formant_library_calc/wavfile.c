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

uint8_t is_valid_wav_header(WaveHeader sample_header)
{
	if (	sample_header.RIFFId 		!= ccRIFF			//'RIFF'
			|| sample_header.fileSize		 < 16			//File size - 8
			|| sample_header.WAVEId 		!= ccWAVE		//'WAVE'
		)
		return 0;
	else
		return 1;

}

uint8_t is_valid_format_chunk(WaveFmtChunk fmt_chunk)
{
	if (	fmt_chunk.fmtId 			== ccFMT				//'fmt '
			&& fmt_chunk.fmtSize 		>= 16					//Format Chunk size is valid

			&&	(
						(	fmt_chunk.audioFormat	== 0x0001		//PCM (int) format
						&& (fmt_chunk.bitsPerSample	== 16 			//16-bit signed
						||	fmt_chunk.bitsPerSample	== 8 			//8-bit unsigned
						||	fmt_chunk.bitsPerSample	== 24			//24-bit signed
						||	fmt_chunk.bitsPerSample	== 32			//32-bit signed
						)
					)
					||
						(	fmt_chunk.audioFormat	== 0x0003		//32-bit float 
						&&	fmt_chunk.bitsPerSample	== 32
						)
					||
						(	fmt_chunk.audioFormat	== 0xFFFE		//Extended format (float) format, 32-bit
						&&	fmt_chunk.bitsPerSample	== 32
						)
				)

			&&	(	fmt_chunk.numChannels 	 == 0x0002			//Stereo or Mono only
				||	fmt_chunk.numChannels 	 == 0x0001)

			&&	(	fmt_chunk.sampleRate 	== 96000			//Only 96k, 48k, and 44.1k allowed		
				||	fmt_chunk.sampleRate	== 48000
				||	fmt_chunk.sampleRate	== 44100)			//Todo: we could add support for 88.2kHz

		)
		return (1);

	else
		return (0);
}

void create_waveheader(WaveHeaderAndChunk *w, uint8_t bitsPerSample, uint8_t numChannels, uint32_t sampleRate, uint32_t numSamples)
{
	uint32_t dataSize = (numSamples*bitsPerSample*numChannels/8);

	if (bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32)
		bitsPerSample = 16;

	if (numChannels != 1 && numChannels != 2)
		numChannels = 2;

	w->wh.RIFFId		= ccRIFF;
	w->wh.fileSize 		= sizeof(WaveHeaderAndChunk) - 8 + dataSize; //size of file, minus 8 for 'RIFFsize'
	w->wh.WAVEId 		= ccWAVE;

	w->fc.fmtId 		= ccFMT;
	w->fc.fmtSize		= 16;
	w->fc.audioFormat	= 1;
	w->fc.numChannels	= numChannels;
	w->fc.sampleRate	= sampleRate;
	w->fc.byteRate		= (sampleRate * numChannels * (bitsPerSample/8)); //sampleRate * blockAlign
	w->fc.blockAlign	= numChannels * (bitsPerSample/8);
	w->fc.bitsPerSample= bitsPerSample;

	w->wc.chunkId	= ccDATA;
	w->wc.chunkSize = dataSize;

}
