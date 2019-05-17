#define ccWAVE  0x45564157
#define ccRIFF  0x46464952
#define ccFMT   0x20746D66
#define ccDATA  0x61746164

typedef struct WaveHeader {
    // Riff Wave Header
    uint32_t RIFFId;
    uint32_t fileSize;
    uint32_t WAVEId;
} WaveHeader;

typedef struct WaveChunk {
    // Data Subchunk
    uint32_t chunkId;
    uint32_t  chunkSize;
} WaveChunk;

typedef struct WaveFmtChunk {
    // Format Subchunk
    uint32_t fmtId;
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
} WaveFmtChunk;

typedef struct WaveHeaderAndChunk {
    WaveHeader wh;
    WaveFmtChunk fc;
    WaveChunk wc;
} WaveHeaderAndChunk;

uint8_t is_valid_wav_header(WaveHeader sample_header);
uint8_t is_valid_format_chunk(WaveFmtChunk fmt_chunk);
void create_waveheader(WaveHeaderAndChunk *w, uint8_t bitsPerSample, uint8_t numChannels, uint32_t sampleRate, uint32_t numSamples);
