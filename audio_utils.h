#ifndef AUDIO_UTILS_H_
#define AUDIO_UTILS_H_

#include <stdint.h>
#include <stddef.h>

void Wav_To_Num(const char *filename, int16_t **samples, unsigned int *num_samples);
void Num_to_wav(const char *filename, int16_t *samples, unsigned int num_samples);
void scale_audio(int16_t *samples, unsigned int num_samples, float scaling_factor);
void reverse_audio(int16_t *samples,  unsigned int num_samples);
void swap_chunks(int16_t *samples, unsigned int num_samples);

#endif /* AUDIO_UTILS_H_ */
