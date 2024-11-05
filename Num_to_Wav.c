#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    char riff[4];            // "RIFF"
    uint32_t overall_size;   // overall size of file in bytes
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    uint32_t fmt_size;      // size of format data
    uint16_t audio_format;   // audio format (PCM = 1)
    uint16_t num_channels;    // number of channels (1 = mono, 2 = stereo)
    uint32_t sample_rate;     // sample rate in Hz
    uint32_t byte_rate;       // SampleRate * NumChannels * BitsPerSample/8
    uint16_t block_align;      // NumChannels * BitsPerSample/8
    uint16_t bits_per_sample;   // bits per sample
    char data[4];            // "data"
    uint32_t data_size;      // size of data
} WAVHeader;

void convert_raw_to_wav(const char *raw_filename, const char *wav_filename,
                         uint32_t sample_rate, uint16_t num_channels,
                         uint16_t bits_per_sample) {
    FILE *raw_file = fopen(raw_filename, "rb");
    if (!raw_file) {
        printf("Could not open raw file %s\n", raw_filename);
        return;
    }

    // Open WAV file for writing
    FILE *wav_file = fopen(wav_filename, "wb");
    if (!wav_file) {
        printf("Could not create wav file %s\n", wav_filename);
        fclose(raw_file);
        return;
    }

    // Calculate header information
    WAVHeader header;
    header.audio_format = 1; // PCM
    header.num_channels = num_channels;
    header.sample_rate = sample_rate;
    header.bits_per_sample = bits_per_sample;
    header.byte_rate = sample_rate * num_channels * (bits_per_sample / 8);
    header.block_align = num_channels * (bits_per_sample / 8);

    // Read raw data size
    fseek(raw_file, 0, SEEK_END);
    header.data_size = ftell(raw_file);
    header.overall_size = sizeof(WAVHeader) - 8 + header.data_size; // Subtract 8 bytes for "RIFF" and overall size

    // Reset file pointer to the beginning
    fseek(raw_file, 0, SEEK_SET);

    // Write WAV header
    memcpy(header.riff, "RIFF", 4);
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    header.fmt_size = 16; // Size of the fmt chunk
    memcpy(header.data, "data", 4);

    fwrite(&header, sizeof(WAVHeader), 1, wav_file);

    // Write raw audio data
    uint8_t *buffer = (uint8_t *)malloc(header.data_size);
    fread(buffer, header.data_size, 1, raw_file);
    fwrite(buffer, header.data_size, 1, wav_file);

    // Clean up
    free(buffer);
    fclose(raw_file);
    fclose(wav_file);

    printf("Converted %s to %s successfully.\n", raw_filename, wav_filename);
}

int main() {
    convert_raw_to_wav("denoise.raw", "output.wav", 44100, 1, 16); // Parameters to be changed as per the audio file being used
    return 0;
}
