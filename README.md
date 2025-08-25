# ReallyCheap Twenty VST3 Plugin

A JUCE-based VST3 audio plugin that emulates vintage character with multiple DSP modules including wobble, distortion, digital degradation, magnetic tape simulation, noise, and space effects. Features a macro control system for intuitive parameter modulation and a fully resizable user interface.

## Features

### DSP Modules
- **Bend**: Wow/flutter effects with depth, rate, sync, and stereo controls
- **Bitcrush**: Bit reduction and sample rate degradation
- **Crunch**: Multi-algorithm distortion with drive and tone
- **Tape**: Magnetic tape simulation with compression and saturation
- **Atmosphere**: Noise generation with age, flutter, and width controls
- **Verb**: Reverb/space effects with mix, time, and tone controls

### User Interface
- **Fully resizable interface** with corner resize handle
- **Dynamic scaling** of all UI elements
- **Custom SVG graphics** with transparent overlay system
- **Real-time visual feedback** on parameter changes
- **Module-specific color schemes** with green/gold theme

### Macro Control System
⚠️ **Note**: The macro control system is experimental and may exhibit unstable behavior. Individual module controls are fully functional.

- Single macro knob for unified parameter modulation
- Musical easing curves across three zones:
  - 0.0-0.3: Subtle sweetening
  - 0.3-0.7: Tasteful degradation + light space
  - 0.7-1.0: Heavy processing

## Build Requirements

- **IDE**: Visual Studio 2022 (Windows)
- **Build System**: CMake
- **Framework**: JUCE (included as submodule)
- **Target**: VST3

## Building

### Windows

```bash
git clone <repository-url>
cd ReallyCheap-Twenty
git submodule update --init --recursive
cmake -B build -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build --config Release --target ReallyCheap-Twenty_VST3
```

The plugin will automatically install to `C:\Program Files\Common Files\VST3\` after building.

### Mac

```bash
git clone <repository-url>
cd ReallyCheap-Twenty
git submodule update --init --recursive
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build --config Release --target ReallyCheap-Twenty_VST3
```

## Project Structure

```
ReallyCheap-Twenty/
├── Source/
│   ├── core/              # Core plugin functionality
│   │   ├── PluginProcessor.cpp/h
│   │   ├── PluginEditor.cpp/h
│   │   ├── MacroController.cpp/h
│   │   ├── Params.cpp/h
│   │   └── Presets.cpp/h
│   ├── dsp/               # DSP modules
│   │   ├── Wobble.cpp/h   # Bend module
│   │   ├── Distort.cpp/h  # Crunch module
│   │   ├── Digital.cpp/h  # Bitcrush module
│   │   ├── Magnetic.cpp/h # Tape module
│   │   ├── Noise.cpp/h    # Atmosphere module
│   │   └── Space.cpp/h    # Verb module
│   └── ui/                # User interface
│       ├── LookAndFeel.cpp/h
│       └── ModulePanels/
├── assets/                # Audio assets and graphics
├── JUCE/                  # JUCE framework (submodule)
├── build/                 # Build output
└── CMakeLists.txt
```

## Performance Notes

- Real-time safe processing (no allocations in audio thread)
- Optimized for <10% CPU usage on modern systems
- All audio assets are embedded in the plugin binary
- Debug builds include diagnostic output for development

## License

This project uses the JUCE framework. Please refer to JUCE licensing terms for distribution requirements.