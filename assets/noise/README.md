# Noise Assets Documentation

## Overview
The Noise module supports layered stereo noise playback from short loopable audio assets. When assets are not available, the module falls back to procedural generation.

## File Naming Convention
Assets must follow this naming scheme:
- `vinyl_*.wav` - Vinyl surface noise, crackles, pops
- `tape_*.wav` - Tape hiss, motor noise
- `hum_*.wav` - Electrical hum (50/60Hz + harmonics)
- `fan_*.wav` - Fan/air conditioning rumble
- `storePA_*.wav` - Store PA system ambience
- `jazzclub_*.wav` - Jazz club atmosphere (crowd, clinks)

Multiple files per type are supported (e.g., `vinyl_01.wav`, `vinyl_02.wav`).

## Technical Requirements

### Format
- **Sample Rate**: 44.1 kHz (will be resampled if needed)
- **Bit Depth**: 24-bit preferred, 16-bit acceptable
- **Channels**: Stereo (mono files will be converted to dual-mono)
- **Duration**: 2-6 seconds per loop (seamlessly loopable)

### Loudness Target
- **Target Level**: -18 LUFS short-term or -20 dB RMS
- **Peak Limit**: -3 dBFS maximum
- Files will be scaled at runtime via the `noiseLevel` parameter

### Size Budget
- **Total Budget**: ≤ 10 MB for all noise assets combined
- **Per File**: Aim for ~1-2 MB per file
- At 44.1kHz/24-bit stereo: ~530 KB per second

## Asset Preparation Guidelines

### Vinyl
- Surface noise with occasional crackles and pops
- High-pass filter around 20Hz to remove rumble
- Sparse transients for authentic character

### Tape
- Consistent hiss in the 2-10kHz range
- Subtle motor noise below 100Hz
- Should loop seamlessly without obvious patterns

### Hum
- Fundamental at 50Hz or 60Hz depending on region
- Include 2nd and 3rd harmonics at reduced levels
- Add slight broadband noise for realism

### Fan
- Low-frequency rumble (20-200Hz emphasis)
- Air band noise (2-4kHz) at lower level
- Consistent level throughout loop

### StorePA
- Midrange-focused ambience (500-2000Hz)
- Very subtle crowd murmur
- Occasional PA artifacts or flutter

### JazzClub
- Low-level crowd murmur
- Occasional glass clinks (sparse)
- Room tone with natural reverb tail

## Loading Assets

### Startup Loading
Assets are loaded from the plugin's data directory:
```cpp
// In PluginProcessor constructor or prepareToPlay:
auto assetsFolder = juce::File::getSpecialLocation(
    juce::File::commonApplicationDataDirectory)
    .getChildFile("ReallyCheap/ReallyCheap-Twenty/NoiseAssets");

Noise::requestAssetPreload(assetsFolder);
```

### Runtime Replacement
Assets can be hot-swapped from the UI:
```cpp
// From message thread (e.g., button callback):
Noise::requestAssetPreload(newFolder);
```

The audio thread will smoothly transition to new assets without glitches.

## Procedural Fallback

When assets are missing, the module generates noise procedurally:

### Vinyl
- Pink noise + band-limiting (20Hz-15kHz)
- Sparse random crackles (0.2-0.5 seconds apart)
- Stereo decorrelation for width

### Tape
- Pink noise through 30Hz-12kHz bandpass
- Slight amplitude modulation for motor wow
- Consistent hiss level

### Hum
- Sine oscillator at 60Hz with harmonics
- 2nd harmonic at -10dB, 3rd at -20dB
- Narrowband filtered noise around fundamental

### Fan
- Low-passed white noise (<500Hz)
- Weak air band (2-4kHz) component
- Static rumble pattern

### StorePA
- Bandpassed pink noise (500-2000Hz)
- 4Hz amplitude flutter for PA character
- Slight stereo imbalance

### JazzClub
- Low-passed pink noise (<1000Hz)
- Sparse transient generator for clinks
- Wide stereo field via decorrelation

## Licensing Requirements

All noise assets must be either:
1. Original recordings created for this project
2. Licensed under permissive terms (CC0, CC-BY, etc.)
3. Purchased with commercial use rights

Include license information in `assets/noise/LICENSES.txt`.

## Example Asset Creation (Conceptual)

```python
# Pseudo-code for creating a vinyl noise asset
import numpy as np
import soundfile as sf

# Generate 4 seconds at 44.1kHz
duration = 4.0
sr = 44100
samples = int(duration * sr)

# Create pink noise base
pink = generate_pink_noise(samples)

# Add crackles
crackles = generate_sparse_impulses(samples, density=0.5)
vinyl_noise = pink * 0.03 + crackles * 0.1

# Apply filtering
vinyl_noise = highpass(vinyl_noise, 20)
vinyl_noise = lowpass(vinyl_noise, 15000)

# Ensure seamless loop
vinyl_noise = crossfade_ends(vinyl_noise, fade_samples=1000)

# Normalize to -18 LUFS
vinyl_noise = normalize_lufs(vinyl_noise, -18)

# Save as stereo with slight decorrelation
stereo = decorrelate_stereo(vinyl_noise, amount=0.1)
sf.write('vinyl_01.wav', stereo, sr, subtype='PCM_24')
```

## Testing Assets

Before deployment, verify:
1. Seamless looping (no clicks at loop points)
2. Correct loudness level
3. No DC offset
4. No clipping or distortion
5. Natural, believable character
6. File size within budget

## Placeholder Assets

For development, the procedural generators serve as placeholders. No binary assets are included in the repository to keep it lightweight. Production assets should be distributed separately or downloaded on first run.