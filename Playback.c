#include <stdio.h>
#include <string.h>
#include <c6x.h>
#include <csl.h>
#include <csl_mcbsp.h>
#include <csl_edma3.h>
#include <csl_intc.h>
#include "fatfs/ff.h"     // FAT file system module

#define AUDIO_BUFFER_SIZE 1024
#define NUM_BUFFERS 2

// Audio buffers
int16_t audioBuffers[NUM_BUFFERS][AUDIO_BUFFER_SIZE];
int currentBuffer = 0;

// FAT file system objects
FATFS fatfs;
FIL file;

// McBSP handle
CSL_McbspHandle hMcbsp;

// EDMA handle
CSL_Edma3ChannelHandle hEdmaTx;

// Function prototypes
void initAudio();
void setupDMA();
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

    // Setup DMA for audio transfer
    setupDMA();

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
    // Initialize McBSP (Multi-Channel Buffered Serial Port)
    CSL_McbspObj mcbspObj;
    hMcbsp = CSL_mcbspOpen(&mcbspObj, CSL_MCBSP_0, NULL, NULL);

    // Configure McBSP (adjust these settings based on your audio format)
    CSL_McbspConfig config;
    CSL_mcbspGetHwSetup(hMcbsp, &config);
    config.srgr1 = 0x00000100; // CLKGDV = 1, FWID = 0
    config.srgr2 = 0x00003F03; // GSYNC = 0, CLKSP = 0, CLKSM = 1, FPER = 0x3F
    config.pcr = 0x00000A00;   // CLKRM = 1, CLKXM = 1
    config.rcr1 = 0x00010000;  // RWDLEN1 = 1 (16-bit)
    config.xcr1 = 0x00010000;  // XWDLEN1 = 1 (16-bit)
    CSL_mcbspHwSetup(hMcbsp, &config);
}

void configureCodec() {
    // Initialize I2C for codec control
    CSL_I2cObj i2cObj;
    hI2c = CSL_i2cOpen(&i2cObj, CSL_I2C_0, NULL, NULL);

    // Configure I2C for communication with codec
    CSL_I2cConfig i2cConfig;
    CSL_i2cGetHwSetup(hI2c, &i2cConfig);
    i2cConfig.mode = CSL_I2C_MODE_MASTER;
    i2cConfig.bitRate = CSL_I2C_BITRATE_100K;
    CSL_i2cHwSetup(hI2c, &i2cConfig);

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
    for (int i = 0; i < sizeof(codecRegs)/sizeof(codecRegs[0]); i++) {
        CSL_i2cWrite(hI2c, 0x18, codecRegs[i], 2, CSL_I2C_START_STOP);
    }
}

void setupDMA() {
    // Initialize EDMA
    CSL_edma3Init(NULL);

    // Open EDMA channel for TX
    hEdmaTx = CSL_edma3ChannelOpen(NULL, CSL_EDMA3_CHA_MCBSP0_TX, NULL, NULL);

    // Configure EDMA parameters
    CSL_Edma3ParamSetup txParams;

    // TX parameters
    txParams.option = CSL_EDMA3_OPT_MAKE(CSL_EDMA3_ITCCH_EN, \
                                         CSL_EDMA3_TCCH_DIS, \
                                         CSL_EDMA3_ITCINT_EN, \
                                         CSL_EDMA3_TCINT_DIS, \
                                         CSL_EDMA3_TCC_NORMAL,\
                                         CSL_EDMA3_FIFOWIDTH_NONE, \
                                         CSL_EDMA3_STATIC_DIS, \
                                         CSL_EDMA3_SYNC_AB, \
                                         CSL_EDMA3_ADDRMODE_INCR, \
                                         CSL_EDMA3_ADDRMODE_CONST);
    txParams.srcAddr = (uint32_t)audioBuffers[currentBuffer];
    txParams.dstAddr = (uint32_t)&hMcbsp->xbuf;
    txParams.aCnt = sizeof(int16_t);
    txParams.bCnt = AUDIO_BUFFER_SIZE;
    txParams.cCnt = 1;
    txParams.srcBidx = sizeof(int16_t);
    txParams.dstBidx = 0;
    txParams.srcCidx = 0;
    txParams.dstCidx = 0;
    txParams.linkAddr = CSL_EDMA3_LINK_NULL;

    // Set up EDMA channel
    CSL_edma3ParamSetup(hEdmaTx, &txParams);

    // Set up interrupt for TX complete
    IRQ_plug(CSL_INTC_EVENTID_EDMA3_0_CC0_INT1, &dmaIsr);
    IRQ_enable(CSL_INTC_EVENTID_EDMA3_0_CC0_INT1);
}

void startPlayback() {
    // Enable EDMA channel
    CSL_edma3ChannelEnable(hEdmaTx);

    // Start McBSP
    CSL_mcbspEnable(hMcbsp, CSL_MCBSP_BOTH_ENABLE);
}

interrupt void dmaIsr() {
    UINT br;

    // Clear EDMA interrupt
    CSL_edma3ClearInterrupt(hEdmaTx);

    // Switch to the other buffer
    currentBuffer = 1 - currentBuffer;

    // Update EDMA source address to the new buffer
    CSL_edma3ParamSetup(hEdmaTx, &((CSL_Edma3ParamSetup){
        .srcAddr = (uint32_t)audioBuffers[currentBuffer],
        // ... (other parameters remain the same)
    }));

    // Read next chunk of audio data into the buffer we just finished playing
    f_read(&file, audioBuffers[1 - currentBuffer], sizeof(audioBuffers[0]), &br);

    // If we've reached the end of the file, seek back to the start of the audio data
    if (br < sizeof(audioBuffers[0])) {
        f_lseek(&file, 44); // Skip WAV header
        f_read(&file, audioBuffers[1 - currentBuffer] + br, sizeof(audioBuffers[0]) - br, &br);
    }

    // Restart EDMA transfer
    CSL_edma3ChannelEnable(hEdmaTx);
}
