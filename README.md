# Work In progress!
## main.c (as of now)
Has the following functions:
- Converts an array of numbers to a .wav file
- Converts a .wav file to an array of numbers
  - Note: The code assumes that the input .wav file is PCM encoded with 16-bit samples.
  - Also, the number of channels must be adjusted, based on the input audio (mono/stereo).
  - For reference used, [click here](https://docs.fileformat.com/audio/wav/)
- A basic audio playback loop implementation.
- Scale, Reverse, Swap chunks of audio samples
