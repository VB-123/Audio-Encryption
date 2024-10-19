#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2
#define BITS_PER_SAMPLE 16
#define HEADER_SIZE 44

typedef struct {
    char chunkID[4];       
    uint32_t chunkSize; 
    char format[4];        
    char subchunk1ID[4];   
    uint32_t subchunk1Size;
    uint16_t audioFormat;  
    uint16_t numChannels;  
    uint32_t sampleRate;   
    uint32_t byteRate; 
    uint16_t blockAlign;   
    uint16_t bitsPerSample;
    char subchunk2ID[4];   
    uint32_t subchunk2Size;
} WavHeader;

void printWavHeader(const WavHeader *header) {
    printf("Chunk ID: %.4s\n", header->chunkID);
    printf("Chunk Size: %u\n", header->chunkSize);
    printf("Format: %.4s\n", header->format);
    printf("Subchunk1 ID: %.4s\n", header->subchunk1ID);
    printf("Subchunk1 Size: %u\n", header->subchunk1Size);
    printf("Audio Format: %u\n", header->audioFormat);
    printf("Number of Channels: %u\n", header->numChannels);
    printf("Sample Rate: %u\n", header->sampleRate);
    printf("Byte Rate: %u\n", header->byteRate);
    printf("Block Align: %u\n", header->blockAlign);
    printf("Bits per Sample: %u\n", header->bitsPerSample);
    printf("Subchunk2 ID: %.4s\n", header->subchunk2ID);
    printf("Subchunk2 Size: %u\n", header->subchunk2Size);
}

void Wav_To_Num(const char *filename, int16_t **samples, size_t *num_samples) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening file.\n");
        return;
    }

    WavHeader header;
    fread(&header, sizeof(WavHeader), 1, file);
    
    if (strncmp(header.chunkID, "RIFF", 4) != 0 || strncmp(header.format, "WAVE", 4) != 0) {
        printf("Not a valid WAV file.\n");
        fclose(file);
        return;
    }
    
    printf("Input file header:\n");
    printWavHeader(&header);
    
    *num_samples = header.subchunk2Size / (header.bitsPerSample / 8);
    printf("Calculated number of samples: %zu\n", *num_samples);

    *samples = (int16_t *)malloc(*num_samples * sizeof(int16_t));
    if (!(*samples)) {
        printf("Memory allocation error.\n");
        fclose(file);
        return;
    }

    size_t samples_read = fread(*samples, sizeof(int16_t), *num_samples, file);
    printf("Actual samples read: %zu\n", samples_read);
    
    fclose(file);
    printf(".wav file '%s' successfully converted to an array.\n", filename);
}

void Num_to_wav(const char *filename, int16_t *samples, size_t num_samples) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error opening file for writing.\n");
        return;
    }

    WavHeader header;
    memcpy(header.chunkID, "RIFF", 4);
    header.chunkSize = 36 + num_samples * (BITS_PER_SAMPLE / 8);
    memcpy(header.format, "WAVE", 4);
    memcpy(header.subchunk1ID, "fmt ", 4);
    header.subchunk1Size = 16;
    header.audioFormat = 1;
    header.numChannels = NUM_CHANNELS;
    header.sampleRate = SAMPLE_RATE;
    header.byteRate = SAMPLE_RATE * NUM_CHANNELS * (BITS_PER_SAMPLE / 8);
    header.blockAlign = NUM_CHANNELS * (BITS_PER_SAMPLE / 8);
    header.bitsPerSample = BITS_PER_SAMPLE;
    memcpy(header.subchunk2ID, "data", 4);
    header.subchunk2Size = num_samples * (BITS_PER_SAMPLE / 8);

    fwrite(&header, sizeof(WavHeader), 1, file);
    
    printf("Output file header:\n");
    printWavHeader(&header);
    
    size_t samples_written = fwrite(samples, sizeof(int16_t), num_samples, file);
    printf("Samples written: %zu\n", samples_written);

    fclose(file);
    printf(".wav file '%s' created successfully.\n", filename);
}

// Scaling
void scale_audio(int16_t *samples, size_t num_samples, float scaling_factor) {
    for (size_t i = 0; i < num_samples; i++) {
        samples[i] = (int16_t)(samples[i] * scaling_factor);
    }
}

// Reverse the audio samples
void reverse_audio(int16_t *samples, size_t num_samples) {
    size_t start = 0;
    size_t end = num_samples - 1;
    
    while (start < end) {
        int16_t temp = samples[start];
        samples[start] = samples[end];
        samples[end] = temp;
        start++;
        end--;
    }
}

// Swap chunks of audio samples
void swap_chunks(int16_t *samples, size_t num_samples) {
    size_t chunk_size = num_samples / 4;
    for (size_t i = 0; i < chunk_size; i++) {
        // Swap the first and last chunks
        int16_t temp = samples[i];
        samples[i] = samples[num_samples - chunk_size + i];
        samples[num_samples - chunk_size + i] = temp;
        
        // Swap the second and third chunks
        temp = samples[chunk_size + i];
        samples[chunk_size + i] = samples[2 * chunk_size + i];
        samples[2 * chunk_size + i] = temp;
    }
}



int main() {
    const char *input_filename = "input.wav";
    int16_t *samples = NULL;
    size_t num_samples = 0;

    Wav_To_Num(input_filename, &samples, &num_samples);
    if (!samples) {
        return 1;
    }

    printf("Original number of samples: %zu\n", num_samples);

    Num_to_wav("output_original.wav", samples, num_samples);

    scale_audio(samples, num_samples, 2.0);
    Num_to_wav("output_scaled.wav", samples, num_samples);

    reverse_audio(samples, num_samples);
    Num_to_wav("output_reversed.wav", samples, num_samples);

    swap_chunks(samples, num_samples);
    Num_to_wav("output_swapped.wav", samples, num_samples);

    free(samples);
    return 0;
}
