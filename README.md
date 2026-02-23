# SputnikMesh

A real-time video synthesis application for openFrameworks featuring mesh-based displacement effects, audio reactivity, and MIDI control.

## Credits

All credit for the original concept goes to **[Andrei Jay](https://andreijaycreativecoding.com/)**. Show him some love and visit his **[Patreon](https://www.patreon.com/andrei_jay)**.

---

## Features

- **Real-time mesh displacement** with shader-based noise modulation
- **Self-contained FFT audio analyzer** - no external FFT library dependencies
- **8-band audio reactivity** with configurable modulation mappings
- **MIDI controller support** for hands-on parameter control
- **Multiple mesh types**: Triangle grid, horizontal/vertical lines, wireframe modes
- **Performance mode** for lower-powered devices (Raspberry Pi, etc.)
- **Fullscreen support** with proper window scaling
- **Save/Load settings** for quick recall of configurations

---

## Quick Start

### Essential Keyboard Shortcuts

| Key | Function |
|-----|----------|
| `?` (Shift+/) | **Show/hide help overlay** - Displays all keyboard shortcuts |
| `Esc` | **Exit application** |
| `` ` `` (backtick) | Toggle debug overlay |
| `F5` | **Save settings** to settings.xml |
| `F9` | **Load settings** from settings.xml |

### Parameter Controls (QWERTY Layout)

**Displacement & Position:**
- `q/w` - X Displace ± | `e/r` - Y Displace ±
- `t/y` - Move Mesh X ± | `u/i` - Move Mesh Y ±
- `o/p` - Zoom ±

**Z Controls:**
- `a/z` - Luma Key ± | `s/x` - Z Frequency ±
- `d/c` - Z LFO Arg ± | `f/v` - Z LFO Amp ±

**X/Y Controls:**
- `g/b` - X Frequency ± | `h/n` - X LFO Arg ± | `j/m` - X LFO Amp ±
- `k/,` - Y Frequency ± | `l/.` - Y LFO Arg ± | `; /` - Y LFO Amp ±

**Toggles (1-8):**
- `1` - Luma Key | `2` - Bright | `3` - Invert | `4` - Wireframe | `5` - B&W
- `6-8` - Cycle LFO Shapes (Z, X, Y)
- `9/0` - Mesh type (Vertical/Horizontal lines)
- `- / =` - Triangle mesh (no wireframe / wireframe)

**Modulation Toggles (!@#$%^):**
- `!` - Z Ring Mod | `@` - X Ring Mod | `#` - Y Ring Mod
- `$` - Z Phase Mod | `%` - X Phase Mod | `^` - Y Phase Mod

### System Controls (Shift+)

| Key | Function |
|-----|----------|
| `Shift+F` | **Toggle fullscreen** |
| `Shift+A` | Toggle audio ON/OFF |
| `Shift+D` | Cycle audio devices |
| `Shift+[/]` | Audio smoothing down/up |
| `Shift+_ / Shift++` | Audio amplitude down/up |
| `Shift+N` | Toggle FFT normalization |
| `Shift+V` | Cycle video device |
| `Shift+C` | Cycle video format |
| `Shift+Z` | Cycle video resolution |
| `Shift+P` | Toggle performance mode |
| `Shift+R` | Reset all parameters |
| `Shift+M` | Generate MIDI mapping template |
| `Shift+I` | Print video info to console |
| `Shift+Ctrl+_ / Shift+Ctrl++` | Decrease/Increase camera FPS |

---

## Configuration Guide

### Settings.xml Structure

The `settings.xml` file contains all configurable parameters:

```xml
<toggles>          <!-- Boolean switches for different features -->
<parameters>       <!-- Numeric values for various effect parameters -->
<lfo>              <!-- Low Frequency Oscillator configurations -->
<transform>        <!-- 3D transformation settings -->
<mesh>             <!-- Mesh rendering options -->
<plocks>           <!-- Parameter lock automation data -->
<midiMappings>     <!-- MIDI controller assignments -->
<audioReactivity>  <!-- Audio input and frequency analysis settings -->
<video>            <!-- Video device and format settings -->
<performance>      <!-- Performance optimization settings -->
```

### Parameter Reference

| Parameter | Description | Keyboard | MIDI CC | Range | XML ID |
|-----------|-------------|----------|---------|-------|--------|
| Luma Key Level | Luma key threshold | A/Z | 16 | 0.0 - 1.0 | luma_key_level |
| X Displacement | Horizontal displacement | Q/W | 17 | -1.0 - 1.0 | x_displace |
| Y Displacement | Vertical displacement | E/R | 18 | -1.0 - 1.0 | y_displace |
| Z Frequency | Z-axis noise frequency | S/X | 19 | 0.0 - 0.1 | z_frequency |
| X Frequency | X-axis noise frequency | G/B | 20 | 0.0 - 0.1 | x_frequency |
| Y Frequency | Y-axis noise frequency | K/, | 21 | 0.0 - 0.1 | y_frequency |
| Zoom | Zoom level | O/P | 22 | 0.0 - 1.0 | zoom |
| Scale/Resolution | Mesh resolution | [/] | - | 50 - 200 | scale |
| Global X Displace | Move mesh horizontally | T/Y | - | -screen to +screen | global_x_displace |
| Global Y Displace | Move mesh vertically | U/I | - | -screen to +screen | global_y_displace |
| Z LFO Argument | Z LFO speed | D/C | - | 0.0 - 1.0 | z_lfo_arg |
| Z LFO Amplitude | Z LFO intensity | F/V | - | 0.0 - 1.0 | z_lfo_amp |
| X LFO Argument | X LFO speed | H/N | - | 0.0 - 1.0 | x_lfo_arg |
| X LFO Amplitude | X LFO intensity | J/M | - | 0.0 - 1.0 | x_lfo_amp |
| Y LFO Argument | Y LFO speed | L/. | - | 0.0 - 1.0 | y_lfo_arg |
| Y LFO Amplitude | Y LFO intensity | ; / | - | 0.0 - 1.0 | y_lfo_amp |

### Toggle Reference

| Toggle | Description | Keyboard | MIDI CC | XML ID |
|--------|-------------|----------|---------|--------|
| Debug Overlay | Show debug info | `` ` `` | - | debug |
| Luma Key | Enable luma key | 1 | 57 | lumaKey |
| Bright Mode | Brightness boost | 2 | 53 | bright |
| Invert | Invert colors | 3 | 54 | invert |
| Wireframe | Wireframe rendering | 4 | - | wireframe |
| Black & White | Monochrome | 5 | 56 | blackAndWhite |
| Z Ring Mod | Z ring modulation | ! | - | zRingMod |
| X Ring Mod | X ring modulation | @ | - | xRingMod |
| Y Ring Mod | Y ring modulation | # | - | yRingMod |
| Z Phase Mod | Z phase modulation | $ | - | zPhaseMod |
| X Phase Mod | X phase modulation | % | - | xPhaseMod |
| Y Phase Mod | Y phase modulation | ^ | - | yPhaseMod |

### MIDI Configuration

Configure MIDI mappings in `settings.xml`:

```xml
<midiMappings>
  <mapping>
    <controlNumber>16</controlNumber>   <!-- MIDI CC number -->
    <channel>0</channel>                <!-- MIDI channel (0-15) -->
    <paramId>luma_key_level</paramId>   <!-- Parameter to control -->
    <minValue>0.0</minValue>            <!-- Minimum value -->
    <maxValue>1.0</maxValue>            <!-- Maximum value -->
    <bipolar>false</bipolar>            <!-- Center at 0 for -1 to 1 range -->
  </mapping>
</midiMappings>
```

### Audio Reactivity Configuration

```xml
<audioReactivity>
  <enabled>1</enabled>
  <amplitude>2.0</amplitude>              <!-- Audio amplification -->
  <smoothing>0.5</smoothing>              <!-- Smoothing factor (0.0-0.99) -->
  <normalization>1</normalization>        <!-- Enable FFT normalization -->
  
  <!-- 8 frequency bands -->
  <bandRanges>
    <range><minBin>0</minBin><maxBin>3</maxBin></range>      <!-- Sub bass -->
    <range><minBin>4</minBin><maxBin>9</maxBin></range>      <!-- Bass -->
    <range><minBin>10</minBin><maxBin>19</maxBin></range>    <!-- Low mids -->
    <range><minBin>20</minBin><maxBin>39</maxBin></range>    <!-- Mids -->
    <range><minBin>40</minBin><maxBin>59</maxBin></range>    <!-- High mids -->
    <range><minBin>60</minBin><maxBin>79</maxBin></range>    <!-- Presence -->
    <range><minBin>80</minBin><maxBin>99</maxBin></range>    <!-- Brilliance -->
    <range><minBin>100</minBin><maxBin>127</maxBin></range>  <!-- Air -->
  </bandRanges>
</audioReactivity>
```

### Video Configuration

```xml
<video>
  <devicePath>/dev/video0</devicePath>   <!-- Device path -->
  <deviceID>0</deviceID>                 <!-- Device index -->
  <format>yuyv422</format>               <!-- Video format -->
  <width>640</width>                     <!-- Capture width -->
  <height>480</height>                   <!-- Capture height -->
  <frameRate>30</frameRate>              <!-- Frame rate -->
</video>
```

### Performance Mode

```xml
<performance>
  <enabled>0</enabled>                    <!-- 0 = off, 1 = on -->
  <scale>50</scale>                       <!-- Mesh resolution in performance mode -->
  <noiseUpdateInterval>4</noiseUpdateInterval> <!-- Update noise every N frames -->
</performance>
```

---

## Building

### macOS

```bash
# Navigate to the project directory
cd /path/to/sputnikMesh

# Build
make -j4

# Run
make RunRelease
# or
./bin/sputnikMesh.app/Contents/MacOS/sputnikMesh
```

### Linux / Raspberry Pi

```bash
# Build
make -j4

# Run
make RunRelease
# or
./bin/sputnikMesh

# For headless operation on Raspberry Pi
DISPLAY=:0 ./bin/sputnikMesh
```

---

## Audio System

This project now uses a **self-contained FFT implementation** (SimpleFFT) based on the Cooley-Tukey algorithm. No external FFT libraries (like ofxFft) are required.

Features:
- 8 frequency bands with logarithmic distribution
- Real-time audio analysis with smoothing
- Per-band attack/release for modulation
- Automatic normalization
- Thread-safe circular buffer for audio input

---

## Troubleshooting

1. **Forgot Key Bindings**: Press `?` anytime to show the help overlay

2. **Invalid XML**: Ensure your XML is properly formatted. Missing closing tags or improper nesting can prevent loading settings.

3. **Device Not Found**: If a device is no longer available, the application will fall back to the default device.

4. **Audio Not Working**: Check that your audio input device is properly connected. Use `Shift+D` to cycle devices.

5. **Video/Camera Issues**:
   - On Linux: Ensure proper permissions (`sudo usermod -a -G video $USER`)
   - Use `v4l2-ctl --list-formats-ext` to see supported formats
   - Check debug overlay (`` ` `` key) for device information

6. **Performance Issues**:
   - Enable performance mode with `Shift+P`
   - Reduce mesh scale with `[` key
   - Use simpler mesh types (9 or 0 for lines)
   - Lower camera resolution in settings.xml

7. **MIDI Not Responding**: Ensure your MIDI device is connected before starting. Check the debug overlay for incoming MIDI messages.

---

## License

See LICENSE file for details.

---

**Note**: This is the `dev` branch. Features and documentation may change as development progresses.
