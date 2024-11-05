#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SAMPLE_RATE 48000
#define DURATION 7
#define LCG_A 1103515245
#define LCG_C 12345
#define LCG_M 0x7FFFFFFF // 31 bits

void encrypt(int32_t *samples, size_t num_samples, uint32_t seed) {
  // Secret seed for the PRNG (must be the same for encryption and decryption)
  size_t i;
  for (i = 0; i < num_samples; i++) {
    // Generate pseudorandom number
    seed = (LCG_A * seed + LCG_C) & LCG_M;
    int32_t rand_value = (int32_t)(seed & 0xFFFF); // Use lower 16 bits

    // Encrypt the sample using XOR
    samples[i] ^= rand_value;
  }
}
int main() {
    size_t num_samples = SAMPLE_RATE*DURATION;
    int32_t *input_samples = (int32_t*)0xC0000000;
    encrypt(input_samples, num_samples, 123456789);
    printf("Encrytped successfully");
    free(input_samples);
    return 0;
}
