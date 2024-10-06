## Understanding Peripherals and their working
1. **EDMA (Enhanced Direct Memory Access)**
**EDMA is a Direct Memory Access (DMA) controller** that moves data between memory and peripherals (like McBSP) without involving the CPU. This is critical for real-time applications like audio playback because it allows the processor to handle other tasks while data is being transferred automatically.

-Why EDMA is used in audio processing:
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
Read audio data from the SD card (using the FATFS library).
Store audio data in memory (in audio buffers).
EDMA transfers the data from the memory buffers to the McBSP.
McBSP transmits the data to the output (audio codec/DAC).
Repeat the process to continuously read new audio data and play it in real-time.
