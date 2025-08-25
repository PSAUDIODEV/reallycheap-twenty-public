# Impulse Response Assets Documentation

## Overview
The Space module uses short stereo impulse responses (IRs) to create spring reverb and room ambience effects. When assets are not available, the module falls back to synthesized IRs.

## File Naming Convention
IRs must follow this naming scheme:
- `spring_*.wav` - Spring reverb IRs (e.g., `spring_a.wav`, `spring_bright.wav`)
- `room_*.wav` - Room/space IRs (e.g., `room_small.wav`, `room_vintage.wav`)

Multiple files per type are supported and will be selected based on the `spaceTime` and `spaceCheapo` parameters.

## Technical Requirements

### Format
- **Sample Rate**: 44.1 kHz (will be resampled if needed)
- **Bit Depth**: 24-bit preferred, 16-bit acceptable
- **Channels**: Stereo (mono files will be converted to dual-mono)
- **Duration**: ≤ 300ms per IR (longer IRs will be truncated)

### Loudness Target
- **Peak Level**: -12 to -18 dBFS recommended
- **Normalization**: Avoid over-normalization; preserve natural dynamics
- IRs will be automatically normalized by the convolution engine

### Size Budget
- **Total Budget**: ≤ 4 MB for all IR assets combined
- **Individual IRs**: Aim for ~500-800 KB per file
- At 44.1kHz/24-bit stereo: ~530 KB per 100ms

## IR Characteristics

### Spring Reverbs
- **Character**: Metallic, resonant, vintage spring tank sound
- **Frequency Response**: Rolled off below 150Hz and above 6kHz
- **Decay**: Quick attack, medium decay (50-200ms typical)
- **Resonances**: Modal resonances around 200Hz, 800Hz, 2kHz
- **Examples**: Real spring tanks, spring emulations, plate reverbs

### Room/Space IRs
- **Character**: Natural room ambience, concert halls, chambers
- **Frequency Response**: Full range, natural acoustics
- **Decay**: Variable decay times (100-300ms for short spaces)
- **Reflections**: Early reflections plus diffuse tail
- **Examples**: Small rooms, booths, echo chambers, live rooms

## Parameter Mapping

### spaceTime (0.1-0.6s)
- Selects IR length or applies windowing to shorten decay
- Shorter times favor punchier, more spring-like IRs
- Longer times prefer room IRs with natural decay

### spaceCheapo (0-1)
- **Low values (0-0.5)**: Prefer room IRs for natural sound
- **High values (0.5-1)**: Prefer spring IRs for vintage coloration
- Additional processing applied: band-limiting, resonance

## Loading IRs

### Startup Loading
IRs are loaded from the plugin's data directory:
```cpp
// In PluginProcessor prepareToPlay() or constructor:
auto irsFolder = juce::File::getSpecialLocation(
    juce::File::commonApplicationDataDirectory)
    .getChildFile("ReallyCheap/ReallyCheap-Twenty/IRs");

Space::requestIRPreload(irsFolder);
```

### Runtime Replacement
IRs can be hot-swapped from the UI without clicks:
```cpp
// From message thread (e.g., file browser):
Space::requestIRPreload(newFolder);
```

The audio thread will crossfade to new IRs over ~20ms for seamless switching.

## Fallback IR Generation

When no assets are present, the module generates synthetic IRs:

### Spring Fallback
- Exponentially decaying white noise burst (250ms)
- Band-limited: 150Hz-6kHz
- Metallic coloration via resonant filtering
- Slight stereo decorrelation

### Room Fallback
- Exponentially decaying noise with natural envelope
- Full-range response: 40Hz-12kHz
- Smooth, non-resonant decay
- Stereo width simulation

## Asset Preparation Guidelines

### Recording Spring Reverbs
1. **Source**: Use real spring tanks, analog spring units
2. **Impulse**: Sharp transient (hand clap, balloon pop)
3. **Capture**: Record full decay until noise floor
4. **Processing**: Minimal - preserve character

### Recording Rooms
1. **Source**: Acoustic spaces with interesting character
2. **Impulse**: Starter pistol, balloon pop, hand clap
3. **Positioning**: Vary mic placement for different sounds
4. **Processing**: High-pass below 40Hz, gentle limiting

### Post-Processing
1. **Trimming**: Remove pre-ring, keep natural attack
2. **Gating**: Gate noise floor but preserve decay tail
3. **EQ**: Minimal corrective EQ only
4. **Stereo**: Ensure true stereo content, avoid mono compatibility issues

## Quality Control

Before deployment, verify each IR:
1. **Duration**: ≤ 300ms after trimming
2. **Level**: Peaks at -12 to -18dBFS
3. **Stereo**: Both channels have content
4. **Artifacts**: No digital artifacts or pre-ring
5. **Character**: Matches intended category (spring/room)
6. **File Size**: Within budget constraints

## Licensing Requirements

All IR assets must be either:
1. **Original recordings** created for this project
2. **Licensed content** with commercial use rights
3. **Public domain** or Creative Commons (CC0, CC-BY)

Include licensing information in `assets/irs/LICENSES.txt` with:
- Source/creator information
- License terms
- Usage restrictions (if any)

## Example IR Creation Workflow

```bash
# Record IR at 48kHz/24-bit
# Import to DAW, trim to 300ms max
# Apply gentle high-pass at 40Hz
# Normalize to -15dBFS peak
# Export as 44.1kHz/24-bit stereo WAV
# Name according to convention
# Verify file size < 800KB
```

## Advanced Usage

### Custom IR Libraries
Users can create custom IR folders:
```
MyCustomIRs/
├── spring_vintage_tank.wav
├── spring_bright_plate.wav
├── room_studio_a.wav
├── room_bathroom.wav
└── LICENSES.txt
```

### Batch Processing
For multiple IRs, consider:
- Consistent peak normalization
- Matched stereo imaging
- Similar character within categories
- Organized naming scheme

## Testing IRs

Manual verification in DAW:
1. Load plugin with IR folder
2. Enable Space module
3. Set `spaceTime` to various values - verify IR selection
4. Adjust `spaceCheapo` - verify spring vs room preference
5. Test `spaceTone` - verify tilt EQ response
6. Check for clicks when changing parameters
7. Verify latency compensation at different mix levels

## Troubleshooting

### Common Issues
- **No reverb**: Check IR folder path and file naming
- **Clicks on parameter change**: Verify IR crossfading
- **Wrong character**: Check spring/room categorization
- **High CPU**: Reduce IR count or shorten length
- **Latency issues**: Check convolution latency reporting

### Debug Output
Enable debug logging to see:
- IR loading status
- File sizes and durations
- Fallback IR usage
- Parameter-based IR selection