#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ti/drv/mcasp/McASP.h>
#include <ti/drv/edma3/EDMA3.h>
#include <ti/drv/i2c/I2C.h>
#include "fatfs/ff.h" // Include FatFS for file operations

#define AUDIO_BUFFER_SIZE 1024
#define NUM_BUFFERS 2

// Audio buffers
int16_t audioBuffers[NUM_BUFFERS][AUDIO_BUFFER_SIZE];
int currentBuffer = 0;

// FatFS objects
FATFS fatfs;
FIL file;

// McASP handle
McASP_Handle hMcasp;

// EDMA handle
EDMA3_Handle hEdmaTx;

// Function prototypes
void initAudio();
void setupEDMA();
void configureCodec();
void startPlayback();
interrupt void dmaIsr();

int main() {
    FRESULT fr;     // FatFs return code
    UINT br;        // File read count

    // Initialize the board support library
    CSL_init();

    // Initialize audio interfaces
    initAudio();

    // Setup EDMA for audio transfer
    setupEDMA();

    // Mount the SD card
    fr = f_mount(&fatfs, "", 1);
    if (fr != FR_OK) {
        printf("Failed to mount SD card\n");
        return 1;
    }

    // Open the audio file
    fr = f_open(&file, "audio.wav", FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open audio file\n");
        return 1;
    }

    // Skip WAV header (assuming 44-byte header)
    f_lseek(&file, 44);

    // Read initial audio data
    f_read(&file, audioBuffers[0], sizeof(audioBuffers[0]), &br);
    f_read(&file, audioBuffers[1], sizeof(audioBuffers[1]), &br);

    // Enable interrupts
    IRQ_globalEnable();

    // Start playback
    startPlayback();

    // Main processing loop
    while(1) {
        // The main processing is handled in the DMA ISR
        // This loop can be used for any non-real-time tasks
    }

    // Clean up (in practice, you'd need a way to break out of the while loop)
    f_close(&file);
    f_unmount("");

    return 0;
}

void initAudio() {
    // Initialize McASP
    McASP_Params mcaspParams;
    McASP_Params_init(&mcaspParams);
    mcaspParams.frameSyncMode = McASP_FRAME_SYNC_MODE;
    mcaspParams.dataFormat = McASP_DATA_FORMAT_I2S; // Set to I2S format
    mcaspParams.wordLength = McASP_WORD_LENGTH_16;  // Set 16-bit word length
    hMcasp = McASP_open(MCASP_INSTANCE, &mcaspParams);

    // Configure additional McASP settings as needed
    // You can configure the clock and frame sync settings here
}

void configureCodec() {
    // Initialize I2C for codec control
    I2C_Params i2cParams;
    I2C_Params_init(&i2cParams);
    I2C_Handle hI2c = I2C_open(0, &i2cParams);

    // Configure TLV320AIC3106 codec (example configuration, adjust as needed)
    uint8_t codecRegs[][2] = {
        {0x00, 0x00}, // Select page 0
        {0x01, 0x01}, // Software reset
        {0x0B, 0x81}, // Power up NDAC and set NDAC=1
        {0x0C, 0x82}, // Power up MDAC and set MDAC=2
        {0x0D, 0x00}, // Power up NADC and set NADC=0
        {0x0E, 0x80}, // Power up MADC and set MADC=0
        {0x12, 0x81}, // BCLK N divider powered up and BCLK N=1
        {0x13, 0x00}, // CODEC_CLKIN = MCLK
        {0x3C, 0x01}, // Set DAC Mode to left data = left DAC and right data = right DAC
        {0x3D, 0x01}, // Set ADC overflow flag = B6 and ADC Mode to left data = left ADC and right data = right ADC
        {0x3F, 0xD6}, // Power up Left and Right DAC data paths and set them to 48 kHz
        {0x40, 0x00}, // Set DAC volume to 0dB
        {0x41, 0x00}, // Set DAC volume to 0dB
        {0x51, 0xC0}, // Power up HPL and HPR
        {0x52, 0x00}  // Unmute HPL and HPR
    };

    // Send configuration to codec
    for (int i = 0; i < sizeof(codecRegs) / sizeof(codecRegs[0]); i++) {
        I2C_Transaction i2cTransaction;
        i2cTransaction.slaveAddress = 0x18; // Codec I2C address
        i2cTransaction.writeBuf = codecRegs[i];
        i2cTransaction.writeCount = 2;
        I2C_transfer(hI2c, &i2cTransaction);
    }
}

void setupEDMA() {
    // Initialize EDMA
    EDMA3_Params edmaParams;
    EDMA3_Params_init(&edmaParams);
    hEdmaTx = EDMA3_open(0, &edmaParams);

    // Configure EDMA parameters
    EDMA3_ChannelHandle hChannel = EDMA3_allocChannel(hEdmaTx, EDMA3_CHANNEL_0);
    
    // Set up the transfer attributes (source, destination, etc.)
    EDMA3_setParam(hChannel, audioBuffers[currentBuffer], (void *)&hMcasp->xbuf, AUDIO_BUFFER_SIZE, sizeof(int16_t), 1, 0);
    
    // Enable the DMA transfer
    EDMA3_start(hChannel);
    
    // Set up interrupt for TX complete
    IRQ_plug(CSL_INTC_EVENTID_EDMA3_0_CC0_INT1, &dmaIsr);
    IRQ_enable(CSL_INTC_EVENTID_EDMA3_0_CC0_INT1);
}

void startPlayback() {
    // Enable McASP
    McASP_start(hMcasp);
}

interrupt void dmaIsr() {
    UINT br;

    // Clear EDMA interrupt
    EDMA3_clearInterrupt(hEdmaTx);

    // Switch to the other buffer
    currentBuffer = 1 - currentBuffer;

    // Read next chunk of audio data into the buffer we just finished playing
    f_read(&file, audioBuffers[1 - currentBuffer], sizeof(audioBuffers[0]), &br);

    // If we've reached the end of the file, seek back to the start of the audio data
    if (br < sizeof(audioBuffers[0])) {
        f_lseek(&file, 44); // Skip WAV header
        f_read(&file, audioBuffers[1 - currentBuffer] + br, sizeof(audioBuffers[0]) - br, &br);
    }

    // Restart EDMA transfer
    EDMA3_start(hEdmaTx);
}
