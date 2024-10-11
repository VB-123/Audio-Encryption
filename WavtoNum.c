#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
    uint16_t totalbytes;   
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
    printf("Total Bytes: %u\n", header->totalbytes);
    printf("Bits per Sample: %u\n", header->bitsPerSample);
    printf("Subchunk2 ID: %.4s\n", header->subchunk2ID);
    printf("Subchunk2 Size: %u\n", header->subchunk2Size);
}

void WavToArray(const char *filename, int16_t **samples, size_t *num_samples) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening file.\n");
        return;
    }

    WavHeader header;
    fread(&header, sizeof(WavHeader), 1, file);
    
    // Verify the format
    if (strncmp(header.chunkID, "RIFF", 4) != 0 || strncmp(header.format, "WAVE", 4) != 0) {
        printf("Not a valid WAV file.\n");
        fclose(file);
        return;
    }
    printWavHeader(&header);
    
    *num_samples = header.subchunk2Size / sizeof(int16_t);

    *samples = (int16_t *)malloc(*num_samples * sizeof(int16_t));
    if (!(*samples)) {
        printf("Memory allocation error.\n");
        fclose(file);
        return;
    }

    fread(*samples, sizeof(int16_t), *num_samples, file);
    fclose(file);
    printf(".wav file '%s' successfully converted to an array.\n", filename);
}

int main() {
    const char *filename = "output.wav";
    int16_t *samples = NULL;
    size_t num_samples = 0;

    WavToArray(filename, &samples, &num_samples);
    printf("Number of samples: %zu\n", num_samples);
    for (size_t i = 0; i < num_samples; ++i) {
        printf("%d\t", samples[i]);
    }
    free(samples);
    return 0;
}
