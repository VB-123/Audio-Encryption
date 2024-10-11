#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 1
#define BITS_PER_SAMPLE 16
#define DURATION 2
#define FREQUENCY 440
void Num_to_wav(const char *filename, int16_t *samples, size_t num_samples){
  FILE *file = fopen(filename, "wb");
  if (!file) {
        printf("Error");
        return;
  }
  //Thw header
  uint32_t chunk_size = 36 + num_samples * (BITS_PER_SAMPLE / 8);
  uint32_t subchunk1_size = 16;
  uint16_t audio_format = 1;
  uint32_t sample_rate = SAMPLE_RATE;
  uint16_t num_channels = NUM_CHANNELS;      
  uint16_t bits_per_sample = BITS_PER_SAMPLE;
  uint32_t byte_rate = SAMPLE_RATE * NUM_CHANNELS * (BITS_PER_SAMPLE / 8);
  uint16_t total_bytes = NUM_CHANNELS * (BITS_PER_SAMPLE / 8);
  uint32_t datachunk_size = num_samples * (BITS_PER_SAMPLE / 8);
  fwrite("RIFF", 1, 4, file); 
  fwrite(&chunk_size, sizeof(uint32_t), 1, file); //Size of overall file - 8 bytes
  fwrite("WAVE", 1, 4, file);
  fwrite("fmt ", 1, 4, file);
  fwrite(&subchunk1_size, sizeof(uint32_t), 1, file);
  fwrite(&audio_format, sizeof(uint16_t), 1, file);
  fwrite(&num_channels, sizeof(uint16_t), 1, file);
  fwrite(&sample_rate, sizeof(uint32_t), 1, file);
  fwrite(&byte_rate, sizeof(uint32_t), 1, file);
  fwrite(&total_bytes, sizeof(uint16_t),1,file);
  fwrite(&bits_per_sample, sizeof(uint16_t), 1, file);
  fwrite("data", 1, 4, file);
  fwrite(&datachunk_size, sizeof(uint32_t), 1, file);
  
  // writing the data
  fwrite(samples, sizeof(int16_t), num_samples, file);
  fclose(file);
  printf(".wav file '%s' created successfully.", filename);
}

int main() {
    size_t num_samples = SAMPLE_RATE * DURATION;
    int16_t samples[num_samples];
    double amplitude = 32767;
    for (size_t i = 0; i < num_samples; i++) {
        double time = (double)i / SAMPLE_RATE;
        samples[i] = (int16_t)(amplitude * sin(2.0 * M_PI * FREQUENCY * time));
    }
    Num_to_wav("output.wav", samples, num_samples);

    return 0;
}
