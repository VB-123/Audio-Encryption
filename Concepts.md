# Understanding main.c (as of now)
- A basic audio playback loop implementation.
- TI-RTOS has its own file system and is FAT compatible (Okay, what?). 
- File Allocation Table (FAT) is a file system that is used in many embedded devices and is supported by almost all operating systems. (Is kinda analogous to a Linked List). Using FAT, we abstract away a lot of low-level file handling.
- For further reading on the FAT, [click here](https://www.tuxera.com/blog/understanding-fat-exfat-file-system/)

# Understanding audio_utils.c
- Converting an array of numbers to a wav file and back
- Note: The code assumes that the input .wav file is PCM encoded with 16-bit samples.
- Also, the number of channels must be adjusted, based on the input audio (mono/stereo).
- For reference used, [click here](https://docs.fileformat.com/audio/wav/)
