#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

#define LCG_A 1103515245
#define LCG_C 12345
#define LCG_M 0x7FFFFFFF // 31 bits

void encrypt(int16_t *samples, size_t num_samples, uint32_t seed) {
  // Secret seed for the PRNG (must be the same for encryption and decryption)
  size_t i;
  for (i = 0; i < num_samples; i++) {
    // Generate pseudorandom number
    seed = (LCG_A * seed + LCG_C) & LCG_M;
    int16_t rand_value = (int16_t)(seed & 0xFFFF); // Use lower 16 bits

    // Encrypt the sample using XOR
    samples[i] ^= rand_value;
  }
}
int main() {
    size_t num_samples = SAMPLE_RATE;
    int16_t *input_samples = (int16_t*)0xC0000000;
    encrypt(input_samples, num_samples, 123456789);
    printf("Encrytped successfully");
    free(input_samples);
    return 0;
}
