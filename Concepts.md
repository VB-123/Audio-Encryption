## Understanding Peripherals and their working
1. **EDMA (Enhanced Direct Memory Access)**
**EDMA is a Direct Memory Access (DMA) controller** that moves data between memory and peripherals (like McBSP) without involving the CPU. This is critical for real-time applications like audio playback because it allows the processor to handle other tasks while data is being transferred automatically.

- Why EDMA is used in audio processing:
  - Efficient data transfer: Audio data must be streamed continuously to avoid gaps or delays. EDMA transfers the audio data from memory to the McBSP (which sends it to the output) in the background, without overloading the CPU.
  - Reduces CPU load: With EDMA, the CPU does not have to move the audio data manually; instead, EDMA automates this. The CPU can focus on other tasks, like managing file input or handling user commands.
  - Real-time requirements: For audio, timing is critical. Using EDMA ensures that data is transferred at a consistent rate, maintaining the flow of audio without delays or glitches.
- How EDMA works in the project:
  - Source and Destination: In the code, the source is the audio buffer in memory where the audio data is stored, and the destination is the McBSP transmit register, which outputs the data.
  - Automating the process: EDMA transfers a chunk of audio data (from the buffer) to the McBSP. When one chunk is done, the interrupt notifies the CPU to load the next chunk into the buffer, while EDMA keeps sending the data to McBSP.

2. **McBSP (Multi-channel Buffered Serial Port)**
McBSP is a serial communication peripheral specifically designed for streaming data, such as audio, between the DSP and external devices (like speakers, headphones, or audio codecs). It supports I2S and TDM formats, which are widely used for audio communication.

- Why McBSP is used in audio processing:
  - Streaming audio data: McBSP is designed for high-speed, continuous data transfer. This makes it ideal for sending audio data in real-time.
  - Supports audio formats: It can handle standard audio formats like I2S, which is commonly used for stereo audio data. It can also handle other formats used for digital audio.
  - Clock and sync control: Audio data often needs precise timing (clock synchronization). McBSP manages the clock signals needed to send audio data at the correct rate (like 44.1 kHz or 48 kHz, typical for audio files).
- How McBSP works in the project:
  - Transmitting audio: McBSP transmits the audio samples from the buffer (which are sent by EDMA) to the codec or DAC (Digital-to-Analog Converter), which then sends the signal to your speakers or headphones.
  - Handling bit streams: Audio data is typically a stream of bits. McBSP serializes the data, transmitting it one bit at a time in sync with a clock signal.
  - Buffering for real-time transfer: McBSP has internal buffers to handle incoming data. The CPU or EDMA loads data into these buffers, and McBSP ensures that the data is transmitted on time, avoiding pauses in the audio stream.

### Simplified Workflow for Audio Playback:
- Read audio data from the SD card (using the FATFS library).
- Store audio data in memory (in audio buffers).
- EDMA transfers the data from the memory buffers to the McBSP.
- McBSP transmits the data to the output (audio codec/DAC).
- Repeat the process to continuously read new audio data and play it in real-time.

3. **I2C (Inter-Integrated Circuit)**
I2C is a serial communication protocol often used to connect low-speed devices like sensors, EEPROMs, and, in this case, an audio codec (a device that converts digital audio to analog signals for playback).

- Why I2C is used in this project:
  - Configuring the audio codec: The TLV320AIC3106 codec on the board needs to be configured via I2C. The codec receives control commands like setting volume, sampling rates, power-up, etc., from the DSP.
  - Control communication: The DSP communicates with the codec using I2C, sending a series of register writes to configure the codec. Once the codec is configured, it will handle the analog audio output.
- How it integrates:
  - DSP → I2C → Codec: The DSP sends commands to the codec over the I2C bus. These commands configure the codec’s internal registers to set up audio playback parameters (e.g., sample rate, audio format, volume).
  - After configuring the codec via I2C, the audio data is transmitted via McBSP (explained earlier) to the codec, where it is converted to an analog signal for the output (speakers, headphone jack, etc.).

4. **SD/MMC Interface**
This is the interface that allows the DSP to read from an SD card, where the audio file (input.wav) is stored. In this project, the SD/MMC peripheral is used to access the SD card using a file system (FAT32) managed by the FATFS library.

- Why the SD/MMC interface is used:
  - File storage: The audio file is stored on an SD card. The SD/MMC peripheral provides the connection between the DSP and the SD card, allowing the DSP to read the audio data.
  - Reading large files: Audio files can be large, so the SD/MMC interface allows the DSP to read small chunks of the file and load them into memory (buffers) for processing.
- How it integrates:
  - DSP → SD/MMC → SD Card: The DSP uses the FATFS library to manage file operations on the SD card. The FATFS library is built on top of the SD/MMC interface, which handles the actual data transfer between the SD card and the DSP.
  - The project opens the input.wav file from the SD card, reads it into memory, and processes it for playback.
5. **Audio Codec (TLV320AIC3106)**
An audio codec is responsible for converting the digital audio data (which the DSP processes) into an analog signal that can be sent to speakers or headphones. In this project, the TLV320AIC3106 codec is configured via I2C and receives audio data from the McBSP.

- Why the audio codec is used:
  - Digital-to-analog conversion: The DSP processes digital audio data (a stream of bits). To output this audio to speakers or headphones, it needs to be converted to an analog signal. The codec handles this conversion.
  - Audio playback features: The codec can also handle other audio-related tasks, like volume control, filtering, and power management.
- How it integrates:
  - DSP → McBSP → Codec → Audio Output: The DSP sends the audio data to the codec via McBSP. The codec then converts this digital audio into an analog signal and sends it to the output device (speakers, headphones, etc.).
  - I2C for control, McBSP for data: While the codec is controlled through I2C (for setting parameters), the actual audio data is sent through McBSP. This keeps control and data paths separate, improving efficiency.
6. **FATFS Library**
The FATFS library is used for file system management on the SD card. It provides functions to mount the file system, open, read, write, and close files stored on the SD card, which is formatted using the FAT (File Allocation Table) file system.

- Why FATFS is used:
  - Access to file system: Audio files like .wav are stored on the SD card. The DSP needs to be able to read from the card, and FATFS allows it to interact with files just like on a typical computer.
  - Handling large files: The library breaks down large files into smaller, manageable chunks that the DSP can load into buffers for processing.
- How it integrates:
  - File reading: The FATFS library is used to open and read the audio file stored on the SD card. It allows the DSP to seek specific positions in the file (like skipping the WAV header) and read chunks of audio data into memory.
  - SD/MMC → FATFS → DSP: The SD/MMC peripheral accesses the card, and the FATFS library allows the DSP to treat the audio file like a regular file stored in memory.
