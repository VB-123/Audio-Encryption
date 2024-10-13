/*
 * audio_utils.h
 *
 *  Created on: 13-Oct-2024
 *      Author: 91884
 */

#ifndef AUDIO_UTILS_H_
#define AUDIO_UTILS_H_

#include <stdint.h>
#include <stddef.h>

void Wav_To_Num(const char *filename, int16_t **samples, size_t *num_samples);
void Num_to_wav(const char *filename, int16_t *samples, size_t num_samples);

#endif /* AUDIO_UTILS_H_ */
