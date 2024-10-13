#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ti/fs/fatfs/ff.h>

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

void scale_audio(int16_t *samples, unsigned int num_samples, float scaling_factor) {
    int16_t i;
    for (i = 0; i < num_samples; i++) {
        samples[i] = (int16_t)(samples[i] * scaling_factor);
    }
}

// Reverse the audio samples
void reverse_audio(int16_t *samples, unsigned int num_samples) {
    unsigned int start = 0;
    unsigned int end = num_samples - 1;

    while (start < end) {
        int16_t temp = samples[start];
        samples[start] = samples[end];
        samples[end] = temp;
        start++;
        end--;
    }
}

// Swap chunks of audio samples
void swap_chunks(int16_t *samples, unsigned int num_samples) {
    int32_t chunk_size = num_samples / 4;
    int32_t i;
    for (i = 0; i < chunk_size; i++) {
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
void Wav_To_Num(const char *filename, int16_t **samples, unsigned int *num_samples) {
    FIL file;
    FRESULT res;
    UINT br;

    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        printf("Error opening file.\n");
        return;
    }

    WavHeader header;
    res = f_read(&file, &header, sizeof(WavHeader), &br);
    if (res != FR_OK || br != sizeof(WavHeader)) {
        printf("Error reading file.\n");
        f_close(&file);
        return;
    }

    if (strncmp(header.chunkID, "RIFF", 4) != 0 || strncmp(header.format, "WAVE", 4) != 0) {
        printf("Not a valid WAV file.\n");
        f_close(&file);
        return;
    }

    *num_samples = header.subchunk2Size / (header.bitsPerSample / 8);

    *samples = (int16_t *)malloc(*num_samples * sizeof(int16_t));
    if (!(*samples)) {
        printf("Memory allocation error.\n");
        f_close(&file);
        return;
    }

    res = f_read(&file, *samples, *num_samples * sizeof(int16_t), &br);
    if (res != FR_OK || br != (*num_samples * sizeof(int16_t))) {
        printf("Error reading samples.\n");
        free(*samples);
        f_close(&file);
        return;
    }

    f_close(&file);
}

void Num_to_wav(const char *filename, int16_t *samples, unsigned int num_samples) {
    FIL file;
    FRESULT res;
    UINT bw;

    res = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
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

    res = f_write(&file, &header, sizeof(WavHeader), &bw);
    if (res != FR_OK || bw != sizeof(WavHeader)) {
        printf("Error writing header.\n");
        f_close(&file);
        return;
    }

    res = f_write(&file, samples, num_samples * sizeof(int16_t), &bw);
    if (res != FR_OK || bw != (num_samples * sizeof(int16_t))) {
        printf("Error writing samples.\n");
        f_close(&file);
        return;
    }

    f_close(&file);
}
