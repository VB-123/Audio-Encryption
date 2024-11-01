# Audio Encryption and Decryption Project for TMDSLCDK6748

## Project Overview
This project implements audio encryption and decryption using a TMS320C6748 DSP processor, with custom encryption algorithms and audio file conversion utilities.

## Repository Structure
```
project-root/
│
├── src/
│   ├── main.c          # Core encryption algorithm implementation
│   └── Num_to_Wav.c    # Numeric array to WAV file conversion
└── README.md
```

## Technical Specifications
- **Processor**: TMS320C6748 (32-bit DSP)
- **Memory**: DDR2 RAM (512 MB)
- **Memory Address Range**: 0xC0000000 to 0xE0000000

## Key Components

### Encryption and Decryption Module (main.c)
- Custom encryption algorithm implementation
- Supports audio data encryption/decryption

### Conversion Utility (Num_to_Wav.c)
- Converts numeric arrays to WAV file format
- Supports .raw file input conversion
- Handles 32-bit word alignment

## Development Environment
- Code Composer Studio (CCS)
- TI Code Generation Tools
- TMS320C6748 DSP Processor Development Kit: TMDSLCDKC6748

## Memory Interface Workflow

### Data Loading
1. Convert source file to binary (.raw)
2. Load binary to DDR2 RAM
   - Start Address: `0xC0000000`
   - Ensure memory alignment
3. Use 32-bit pointer for data access

### Memory Saving
1. Use CCS Save Memory function
2. Calculate word count: `words = file_size_bytes ÷ 4`
3. Save processed data to specified location

## Contributors
- Vasanthi[https://github.com/VB-123]
- Arjun[https://github.com/officiallyaninja]