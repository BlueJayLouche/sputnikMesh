# SputnikMesh Configuration Guide

This document explains how to configure SputnikMesh through the `settings.xml` file. You can adjust MIDI mappings, audio reactivity, FFT bin configuration, video settings, performance mode and more without changing the source code.

## Understanding settings.xml Structure

The `settings.xml` file contains several main sections:

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

## 1. Configuring MIDI Channel Mappings

The `<midiMappings>` section allows you to map MIDI controllers to specific parameters. Here's how to configure it:

```xml
<midiMappings>
  <mapping>
    <controlNumber>16</controlNumber>   <!-- MIDI CC number -->
    <channel>0</channel>                <!-- MIDI channel (0-15) -->
    <paramId>luma_key_level</paramId>   <!-- Parameter to control -->
    <minValue>0.0</minValue>            <!-- Minimum parameter value -->
    <maxValue>1.0</maxValue>            <!-- Maximum parameter value -->
    <bipolar>false</bipolar>            <!-- Center at 0 for -1 to 1 range -->
  </mapping>
  <!-- Add more mappings as needed -->
</midiMappings>
```

### Available Parameter IDs:

Here are the parameter IDs you can use in your mappings:

- `luma_key_level` - Controls luma key threshold (0.0-1.0)
- `x_displace` - Horizontal displacement (-1.0-1.0, typically bipolar)
- `y_displace` - Vertical displacement (-1.0-1.0, typically bipolar)
- `z_frequency` - Z-axis noise frequency (0.0-0.1)
- `x_frequency` - X-axis noise frequency (0.0-0.1)
- `y_frequency` - Y-axis noise frequency (0.0-0.1)
- `zoom` - Zoom level (0.0-1.0)
- `scale` - Mesh resolution (50-200)
- `center_x` - Center X offset (-1.0-1.0, typically bipolar)
- `center_y` - Center Y offset (-1.0-1.0, typically bipolar)
- `z_lfo_arg` - Z LFO argument (0.0-1.0)
- `z_lfo_amp` - Z LFO amplitude (0.0-1.0)
- `x_lfo_arg` - X LFO argument (0.0-1.0)
- `x_lfo_amp` - X LFO amplitude (0.0-1.0)
- `y_lfo_arg` - Y LFO argument (0.0-1.0)
- `y_lfo_amp` - Y LFO amplitude (0.0-1.0)
- `rotate_x` - X-axis rotation (-180.0-180.0, typically bipolar)
- `rotate_y` - Y-axis rotation (-180.0-180.0, typically bipolar)
- `rotate_z` - Z-axis rotation (-180.0-180.0, typically bipolar)
- `global_x_displace` - Global X displacement (-1.0-1.0, typically bipolar)
- `global_y_displace` - Global Y displacement (-1.0-1.0, typically bipolar)
- `stroke_weight` - Line thickness (0.5-5.0)

## 2. Configuring Audio Reactivity and FFT Bins

The `<audioReactivity>` section controls audio input and analysis:

```xml
<audioReactivity>
  <enabled>1</enabled>                   <!-- 0 = disabled, 1 = enabled -->
  <sensitivity>2.000000000</sensitivity> <!-- Amplification of audio input -->
  <smoothing>0.800000012</smoothing>     <!-- Smoothing factor (0.0-0.99) -->
  <numBands>8</numBands>                 <!-- Number of frequency bands -->
  <deviceName>Built-in Microphone</deviceName> <!-- Audio input device -->
  <deviceIndex>0</deviceIndex>           <!-- Device index in system list -->
  <normalizationEnabled>1</normalizationEnabled> <!-- Enable FFT normalization -->
  
  <!-- Frequency band configuration -->
  <bandRanges>
    <range>
      <minBin>0</minBin>     <!-- Starting FFT bin -->
      <maxBin>3</maxBin>     <!-- Ending FFT bin -->
    </range>
    <!-- more ranges... -->
  </bandRanges>
  
  <!-- Parameter mappings for audio reactivity -->
  <mappings>
    <mapping>
      <band>6</band>                 <!-- Which frequency band (0-7) -->
      <paramId>x_displace</paramId>  <!-- Parameter to modulate -->
      <scale>2.000000000</scale>     <!-- Scaling factor -->
      <min>-0.500000000</min>        <!-- Minimum output value -->
      <max>0.500000000</max>         <!-- Maximum output value -->
      <additive>0</additive>         <!-- 0 = replace, 1 = add to value -->
    </mapping>
    <!-- more mappings... -->
  </mappings>
</audioReactivity>
```

### Understanding FFT Bins:

The FFT (Fast Fourier Transform) bins represent different frequency ranges. With the default 1024 FFT size and 44100Hz sample rate, each bin represents about 43Hz. The `<bandRanges>` section maps groups of bins to logical frequency bands.

Default frequency bands:
1. Band 0 (bins 0-3): Sub bass (20-60Hz)
2. Band 1 (bins 4-9): Bass (60-250Hz)
3. Band 2 (bins 10-19): Low mids (250-500Hz)
4. Band 3 (bins 20-39): Mids (500-2000Hz)
5. Band 4 (bins 40-59): High mids (2-4kHz)
6. Band 5 (bins 60-79): Presence (4-6kHz)
7. Band 6 (bins 80-99): Brilliance (6-12kHz)
8. Band 7 (bins 100-127): Air (12-20kHz)

Adjust these ranges to focus on specific frequency areas that you want to react to.

### Audio Normalization

The `<normalizationEnabled>` tag controls whether FFT values are normalized relative to the maximum value:

- When enabled (`1`), the audio reactivity will be consistent regardless of the overall volume level
- When disabled (`0`), the audio reactivity will directly reflect the raw audio levels, showing more variation between quiet and loud sections

## 3. Configuring Video Devices and Formats

The `<video>` section allows you to configure video input settings:

```xml
<video>
  <devicePath>/dev/video0</devicePath>   <!-- Device path (Linux) or device:// (macOS) -->
  <deviceID>0</deviceID>                 <!-- Device index in system list -->
  <format>yuyv422</format>               <!-- Video format (e.g., yuyv422, mjpeg) -->
  <width>640</width>                     <!-- Desired capture width -->
  <height>480</height>                   <!-- Desired capture height -->
  <frameRate>30</frameRate>              <!-- Desired frame rate -->
</video>
```

### Available Video Formats:

Depending on your camera, these formats may be available:
- `yuyv422` or `YUYV` - Common uncompressed format (default)
- `mjpeg` or `MJPG` - Motion JPEG (often higher resolution at good framerates)
- `rgb565` - RGB format
- `bayer_rggb8`, `bayer_bggr8`, etc. - Various Bayer pattern formats
- `h264` - H.264 compressed video (if supported by camera)

On Raspberry Pi with V4L2 support, the actual available formats will be printed to the console when starting the application.

## 4. Performance Mode

The `<performance>` section lets you configure the performance optimization settings:

```xml
<performance>
  <enabled>0</enabled>                    <!-- 0 = off, 1 = on -->
  <scale>50</scale>                       <!-- Mesh resolution in performance mode (lower = faster) -->
  <noiseUpdateInterval>4</noiseUpdateInterval> <!-- Update noise every N frames -->
  <highQuality>1</highQuality>            <!-- 1 = high quality, 0 = optimize for speed -->
</performance>
```

Performance mode applies multiple optimizations for lower-powered devices like Raspberry Pi:
- Reduces mesh resolution
- Updates noise textures less frequently
- Uses smaller noise textures
- Processes fewer pixels in noise generation
- Limits camera resolution and framerate
- Reduces audio processing frequency

Toggle performance mode with `Shift+P` during runtime.

## 5. Selecting Audio Devices

To change the audio input device:

1. Find the `<deviceName>` and `<deviceIndex>` tags in the `<audioReactivity>` section
2. Set `<deviceName>` to the exact name of your audio device (e.g., "Built-in Microphone")
3. Set `<deviceIndex>` to the device's index in your system (typically 0 for default device)

Example:
```xml
<deviceName>Apple Inc.: Built-in Microphone</deviceName>
<deviceIndex>0</deviceIndex>
```

To see available audio devices, run the application and check the console output, which lists all detected audio input devices.

## 6. Toggle Settings

The `<toggles>` section controls various rendering options:

```xml
<toggles>
    <debug>0</debug>             <!-- Enable debug overlay: 0 = off, 1 = on -->
    <wireframe>1</wireframe>     <!-- Wireframe rendering: 0 = off, 1 = on -->
    <bright>1</bright>           <!-- Brightness boost: 0 = off, 1 = on -->
    <invert>1</invert>           <!-- Invert colors: 0 = off, 1 = on -->
    <strokeWeight>0</strokeWeight> <!-- Use custom stroke weight: 0 = off, 1 = on -->
    <blackAndWhite>0</blackAndWhite> <!-- Monochrome: 0 = off, 1 = on -->
    <lumaKey>0</lumaKey>         <!-- Luma key effect: 0 = off, 1 = on -->
</toggles>
```

## 7. Core Parameters

The `<parameters>` section controls primary visual effects:

```xml
<parameters>
    <lumaKeyLevel>0.0</lumaKeyLevel> <!-- Luma key threshold (0.0-1.0) -->
    <xDisplace>0.0</xDisplace>      <!-- X displacement (-1.0-1.0) -->
    <yDisplace>0.0</yDisplace>      <!-- Y displacement (-1.0-1.0) -->
    <zFrequency>0.02</zFrequency>   <!-- Z noise frequency (0.0-0.1) -->
    <xFrequency>0.015</xFrequency>  <!-- X noise frequency (0.0-0.1) -->
    <yFrequency>0.02</yFrequency>   <!-- Y noise frequency (0.0-0.1) -->
    <zoom>0.064</zoom>              <!-- Mesh zoom factor (0.0-1.0) -->
    <scale>100</scale>              <!-- Mesh resolution/density (50-200) -->
    <centerX>0.0</centerX>          <!-- X center offset (-1.0-1.0) -->
    <centerY>0.0</centerY>          <!-- Y center offset (-1.0-1.0) -->
    <!-- Additional LFO parameters -->
</parameters>
```

## 8. LFO and Modulation Settings

The `<lfo>` section controls modulation effects:

```xml
<lfo>
    <yShape>0</yShape>          <!-- Y LFO wave shape: 0=sine, 1=square, 2=triangle, 3=sawtooth -->
    <xShape>0</xShape>          <!-- X LFO wave shape -->
    <zShape>0</zShape>          <!-- Z LFO wave shape -->
    <yRingMod>0</yRingMod>      <!-- Y ring modulation: 0=off, 1=on -->
    <xRingMod>0</xRingMod>      <!-- X ring modulation -->
    <zRingMod>0</zRingMod>      <!-- Z ring modulation -->
    <yPhaseMod>0</yPhaseMod>    <!-- Y phase modulation: 0=off, 1=on -->
    <xPhaseMod>0</xPhaseMod>    <!-- X phase modulation -->
    <zPhaseMod>0</zPhaseMod>    <!-- Z phase modulation -->
</lfo>
```

## Advanced Tips

### Generating a MIDI Mapping Template

The application includes a feature to generate a template MIDI mapping file. Press Shift+M while running the application to generate a `mapping_template.xml` file in the data folder. This template contains all available parameters with default settings, which you can customize and then copy into your settings.xml file.

### Runtime Controls

While running the application, you can use these keyboard shortcuts:

- **\`** (backtick): Toggle debug overlay
- **Shift+R**: Reset all parameters to defaults
- **Shift+A**: Toggle audio reactivity
- **Shift+D**: Cycle through audio input devices
- **Shift+[** / **Shift+]**: Decrease/increase audio smoothing
- **Shift+-** / **Shift+=**: Decrease/increase audio sensitivity
- **Shift+N**: Toggle FFT normalization
- **Shift+P**: Toggle performance mode
- **Shift+V**: Cycle through video devices
- **Shift+F**: Cycle through video formats
- **Shift+Ctrl+R**: Cycle through video resolutions
- **Shift+Ctrl+-** / **Shift+Ctrl+=**: Decrease/increase camera framerate
- **Shift+I**: Print video device information to console
- **1-8**: Toggle various effects (Luma Key, Bright, Invert, etc.)
- **9-0**: Change mesh types
- **[** / **]**: Decrease/increase mesh resolution

## Raspberry Pi Optimization

For best performance on Raspberry Pi:

1. Enable performance mode with `Shift+P` or set it in settings.xml: `<performance><enabled>1</enabled></performance>`
2. Choose MJPEG format if available for your camera (`Shift+F` to cycle formats)
3. Use lower resolutions (640x480 or 320x240)
4. Set a lower framerate (24fps is usually sufficient)
5. When running headless, launch with: `DISPLAY=:0 ./SputnikMesh`

## Troubleshooting

1. **Invalid XML**: Ensure your XML is properly formatted. Missing closing tags or improper nesting can prevent the application from loading settings.

2. **Device Not Found**: If an audio or video device is no longer available, the application will fall back to the default device.

3. **Parameter Out of Range**: If you set values outside the expected ranges, the application will clamp them to valid values.

4. **Missing Sections**: If you remove entire sections from settings.xml, the application will use default values for those settings.

5. **Audio Not Working**: Check that your audio input device is properly connected and that the correct device is selected in settings.xml.

6. **Video/Camera Issues**: 
   - On Linux/Raspberry Pi, ensure you have proper permissions for the video device (`sudo usermod -a -G video $USER`)
   - Check if the format and resolution are supported by your camera
   - Try using `v4l2-ctl --list-formats-ext` in a terminal to see supported formats
   - In debug mode (press \` key), check the video device information

7. **MIDI Not Responding**: Ensure your MIDI device is connected before starting the application. Check the debug overlay to confirm MIDI messages are being received.

8. **Performance Issues**:
   - Enable performance mode with `Shift+P`
   - Reduce mesh scale/resolution
   - Use simpler mesh types (horizontal or vertical lines)
   - Disable audio reactivity if not needed
   - Lower the camera resolution and framerate
