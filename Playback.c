#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ti/fs/fatfs/ff.h>

#define AUDIO_BUFFER_SIZE 512
#define BLOCK_SIZE 256

void audio_loopback(const char *input_filename, const char *output_filename) {
    FATFS fatfs;
    FIL input_file, output_file;
    FRESULT res;
    UINT bytes_read, bytes_written;
    uint8_t buffer[AUDIO_BUFFER_SIZE];

    res = f_mount(&fatfs, "", 1);
    if (res != FR_OK) {
        printf("Error mounting the filesystem: %d\n", res);
        return;
    }

    res = f_open(&input_file, input_filename, FA_READ);
    if (res != FR_OK) {
        printf("Error opening input file: %d\n", res);
        f_mount(0, "", 0);  // Unmount in case of error
        return;
    }

    res = f_open(&output_file, output_filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        printf("Error opening output file: %d\n", res);
        f_close(&input_file);
        f_mount(0, "", 0);  // Unmount in case of error
        return;
    }

    while ((res = f_read(&input_file, buffer, AUDIO_BUFFER_SIZE, &bytes_read)) == FR_OK && bytes_read > 0) {
        res = f_write(&output_file, buffer, bytes_read, &bytes_written);
        if (res != FR_OK || bytes_written < bytes_read) {
            printf("Error writing to output file: %d\n", res);
            break;
        }
    }

    f_close(&input_file);
    f_close(&output_file);
    f_mount(0, "", 0);

    printf("Loopback complete.\n");
}

int main(){
    //printf("Hello world");
    audio_loopback("input.wav", "output.wav");
}
