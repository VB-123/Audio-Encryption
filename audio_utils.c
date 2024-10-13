#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

    *num_samples = header.subchunk2Size / (header.bitsPerSample / 8);

    *samples = (int16_t *)malloc(*num_samples * sizeof(int16_t));
    if (!(*samples)) {
        printf("Memory allocation error.\n");
        fclose(file);
        return;
    }

    fread(*samples, sizeof(int16_t), *num_samples, file);
    fclose(file);
}

void Num_to_wav(const char *filename, int16_t *samples, size_t num_samples) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error opening file for writing.\n");
        return;
    }

    WavHeader header;
    memcpy(header.chunkID, "RIFF", 4);
    header.chunkSize = 36 + num_samples * (16 / 8);
    memcpy(header.format, "WAVE", 4);
    memcpy(header.subchunk1ID, "fmt ", 4);
    header.subchunk1Size = 16;
    header.audioFormat = 1;
    header.numChannels = 2;
    header.sampleRate = 44100;
    header.byteRate = 44100 * 2 * (16 / 8);
    header.blockAlign = 2 * (16 / 8);
    header.bitsPerSample = 16;
    memcpy(header.subchunk2ID, "data", 4);
    header.subchunk2Size = num_samples * (16 / 8);

    fwrite(&header, sizeof(WavHeader), 1, file);
    fwrite(samples, sizeof(int16_t), num_samples, file);
    fclose(file);
}
