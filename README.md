# Teensy 4.1 Synth V1

A polyphonic audio synthesizer built on the **Teensy 4.1** microcontroller with USB MIDI support, featuring advanced audio processing and real-time parameter control.

## Overview

This is a comprehensive synthesis engine running on the Teensy 4.1 platform, utilizing the powerful Audio Library to deliver complex sound synthesis capabilities. The synth supports 8-voice polyphony, multiple oscillator types, dual filter modes, envelope modulation, effects processing, and custom waveform management.

## Hardware

- **Microcontroller**: Teensy 4.1
- **Audio Interface**: SGTL5000 audio codec (I2S connectivity)
- **USB MIDI**: Support for dual USB MIDI inputs (device MIDI and USB host)
- **I/O Indicators**: 
  - 8 LED chain (shift register controlled via 74HC595)
  - VU meter output (PWM controlled)
  - Overdrive indicator LED
- **Analog Control**: Multiplexed analog inputs for synthesis parameters
- **Keyboard Input**: USB MIDI keyboard support via USB host

## Architecture

### Core Modules

#### Voice System (`src/voice/`)
- **Voice.h/cpp**: Individual voice synthesizer with:
  - 7 modulated oscillators
  - Dual LFO system (3 enveloped LFOs + 1 free-running LFO)
  - Noise generator
  - Dual filter types (Ladder and State Variable)
  - Multi-tap delay effect
  - Voice envelope with attack, decay, sustain, release
  - Overdrive detection across signal chain

- **VoiceController.h/cpp**: Manages 8-voice polyphony:
  - Note on/off handling
  - Voice allocation and recycling
  - Stereo output mixing

#### Input/Output System (`src/io/`)
- **InputController.h**: Central input management
  - Analog multiplexer handling
  - MIDI input processing
  - Parameter mapping to synthesis engine

- **SynthConfigurationMapper.h**: Maps user inputs to synthesis parameters
  - Comprehensive parameter routing
  - Real-time parameter update handling

- **Indicators.h**: Visual feedback system
  - LED chain animation control
  - VU meter management
  - Voice count display
  - Waveform selection indicators

- **MidiIo.h**: MIDI event handlers
- **MuxIo.h**: Multiplexed analog input management
- **AnalogBuffer.h**: Analog input buffering

#### Configuration System (`src/config/`)
- **SynthConfiguration.h**: Stores all synthesis parameters
  - Oscillator configurations
  - Filter settings
  - Envelope parameters
  - Effect amounts

- **SynthConfigurationListener.h**: Observer pattern for parameter changes

#### Storage System (`src/store/`)
- **PresetStore.h**: Preset save/load functionality
  - 128 preset slots
  - MIDI-controlled preset selection

- **WaveformStore.h**: Custom waveform management
  - SysEx upload support
  - Waveform selection and playback
  - 6 custom waveform slots

#### Audio Processing (`src/audio/`)
- **AudioAnalyzeOverdrive.h**: Overdrive detection utility

### Main Application (`src/main.cpp`)
The main application orchestrates:
- SGTL5000 codec initialization
- Audio memory allocation (1200 blocks)
- USB MIDI device setup (dual USB MIDI inputs)
- Main synthesis loop coordination
- Keyboard connection monitoring

## Features

### Synthesis
- **8-Voice Polyphony**: Simultaneous note playback
- **7 Oscillators Per Voice**: Flexible oscillator modulation control
- **Dual Filter Types**: 
  - Ladder filter (classic analog character)
  - State Variable filter (modern versatility)
- **3-Envelope LFO System**: Dedicated LFOs for modulation shaping
- **Noise Generator**: White noise for texture
- **Effects**: 
  - Multi-tap delay per voice
  - Overdrive detection and indication

### Control
- **USB MIDI**: 
  - Note on/off
  - Pitch bend
  - CC parameter control
  - SysEx waveform uploads
- **Analog Controls**: Real-time parameter adjustment via multiplexed inputs
- **Preset Management**: 128 preset slots with SysEx support
- **Custom Waveforms**: Upload and manage 6 custom waveforms via SysEx

### User Feedback
- **LED Indicators**: 8-LED chain showing:
  - Voice activity
  - Waveform selection
  - Keyboard connection status
- **Overdrive LED**: Alert when signal clipping is detected
- **VU Meter**: Real-time output level display

## Building & Deployment

### Requirements
- PlatformIO IDE or CLI
- Teensy Loader
- Arduino framework for Teensy

### Configuration
Edit `platformio.ini`:
```ini
[env:teensy41]
platform = teensy
board = teensy41
framework = arduino
build_flags = -D USB_MIDI_SERIAL
```

### Build & Upload
```bash
platformio run -e teensy41 -t upload
```

### Linux Setup
Copy udev rules for Teensy device access:
```bash
sudo cp 00-teensy.rules /etc/udev/rules.d/00-teensy.rules
```

## Project Structure

```
synth-v1/
├── src/
│   ├── main.cpp                           # Application entry point
│   ├── voice/
│   │   ├── Voice.h/cpp                    # Individual voice synth
│   │   ├── VoiceController.h/cpp          # Polyphony management
│   ├── io/
│   │   ├── InputController.h              # Input handling
│   │   ├── Indicators.h                   # LED/VU feedback
│   │   ├── SynthConfigurationMapper.h     # Parameter mapping
│   │   ├── MidiIo.h                       # MIDI interface
│   │   ├── MuxIo.h                        # Analog multiplexer
│   │   └── AnalogBuffer.h                 # Analog buffering
│   ├── config/
│   │   ├── SynthConfiguration.h           # Parameter storage
│   │   └── SynthConfigurationListener.h   # Change observer
│   ├── store/
│   │   ├── PresetStore.h                  # Preset management
│   │   └── WaveformStore.h                # Custom waveforms
│   ├── audio/
│   │   └��─ AudioAnalyzeOverdrive.h        # Overdrive detection
│   └── utils/                             # Utility functions
├── lib/                                   # External libraries
├── waveforms/
│   └── sine.wav                           # Default waveform
├── platformio.ini                         # Build configuration
├── 00-teensy.rules                        # Linux udev rules
└── README.md                              # This file
```

## Audio Chain

Each voice processes audio through:
```
Oscillators (7) + Noise
    ↓
Oscillator Mixer
    ↓
Filter (Ladder or SVF)
    ↓
Voice Envelope
    ↓
Delay Effect
    ↓
Output Mixer
```

Modulation is applied via:
- **LFO 1a, 1b, 1c**: Enveloped oscillators modulating specific voices
- **LFO 2**: Free-running filter modulation

## Getting Started

1. **Program the Teensy 4.1** with this firmware
2. **Connect USB MIDI** keyboard or controller
3. **Monitor LEDs** for keyboard connection status
4. **Use MIDI CC** to adjust synthesis parameters in real-time
5. **Load presets** using MIDI program change
6. **Upload custom waveforms** via SysEx

## MIDI Implementation

### Note Events
- **Note On (90 00 00 - 7F)**: Trigger note playback
- **Note Off (80 00 00 - 7F)**: Release note

### Control Changes
Route to synthesis parameters via `SynthConfigurationMapper`

### System Exclusive
- Preset management
- Custom waveform uploads (6 slots available)
- Configuration backup/restore

## Performance

- **Audio Buffer**: 1200 blocks allocated
- **CPU Monitoring**: Optional real-time CPU/memory stats
- **Sample Rate**: 44.1 kHz (Teensy 4.1 default)

## Future Enhancements

- Additional effect types (reverb, chorus)
- Enhanced preset browser UI
- OLED/LCD display integration
- Sequencer functionality
- Arpeggiator modes

## License

[Specify your license here]

## Contributing

[Contribution guidelines]

## Support

For issues and questions, please open an issue on the GitHub repository.
