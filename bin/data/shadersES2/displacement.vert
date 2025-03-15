// Uniform matrices
uniform mat4 modelViewProjectionMatrix;

// Input attributes
attribute vec4 position;
attribute vec2 texcoord;

// Output varying
varying vec2 texCoordVarying;

// Texture samplers
uniform sampler2D tex0;
uniform sampler2D x_noise_image;
uniform sampler2D y_noise_image;
uniform sampler2D z_noise_image;

// Displacement controls
uniform vec2 xy;
uniform vec2 xy_offset;

// LFO parameters
uniform float x_lfo_arg;
uniform float y_lfo_arg;
uniform float z_lfo_arg;
uniform float x_lfo_amp;
uniform float y_lfo_amp;
uniform float z_lfo_amp;
uniform float x_lfo_other;
uniform float y_lfo_other;
uniform float z_lfo_other;

// LFO shape controls
uniform int xLfoShape;
uniform int yLfoShape;
uniform int zLfoShape;

// Modulation toggles
uniform int x_phasemod_switch;
uniform int y_phasemod_switch;
uniform int z_phasemod_switch;
uniform int x_ringmod_switch;
uniform int y_ringmod_switch;
uniform int z_ringmod_switch;

// Display controls
uniform int bright_switch;
uniform int width;
uniform int height;

/**
 * Generate oscillation based on selected waveform shape
 * Modified to accept texture coordinates as a parameter
 */
float oscillate(float theta, int shape, int xyz, vec2 texCoords) {
    float osc = 0.0;
    
    // Waveform selection
    if (shape == 0) {
        // Sine wave
        osc = sin(theta);
    } else if (shape == 1) {
        // Square wave
        osc = sign(sin(theta));
    } else if (shape == 2) {
        // Sawtooth wave
        osc = fract(theta / 6.28) * 2.0 - 1.0;
    } else if (shape == 3) {
        // Perlin noise from texture
        if (xyz == 0) {
            osc = 2.0 * (texture2D(x_noise_image, texCoords * 0.5).r - 0.5);
        } else if (xyz == 1) {
            osc = 2.0 * (texture2D(y_noise_image, texCoords * 0.5).r - 0.5);
        } else {
            osc = 2.0 * (texture2D(z_noise_image, texCoords * 0.5).r - 0.5);
        }
    }
    
    return osc;
}

void main() {
    // Pass texture coordinates to fragment shader
    texCoordVarying = texcoord;

    // Calculate base position
    vec4 newPosition = modelViewProjectionMatrix * position;
    
    // Get brightness from texture
    vec4 color = texture2D(tex0, texCoordVarying);
    float bright = 0.33 * color.r + 0.5 * color.g + 0.16 * color.b;
    
    // Adjust brightness with logarithmic curve for more natural feel
    bright = 2.0 * log(1.0 + bright);
    
    // Apply brightness inversion if enabled
    if (bright_switch == 1) {
        bright = 1.0 - bright;
    }
   
    // Center coordinates for displacement
    newPosition.x += xy_offset.x;
    newPosition.y += xy_offset.y;
     
    // ===== First oscillator pass =====
    
    // X oscillator (first pass)
    float xLfo = x_lfo_amp * oscillate(x_lfo_arg + newPosition.y * x_lfo_other, xLfoShape, 0, texCoordVarying);
    
    // Y oscillator with possible ring modulation from X
    float yLfoAmp = y_lfo_amp;
    if (y_ringmod_switch == 1) yLfoAmp += 0.01 * xLfo;
    
    float yLfoPhase = y_lfo_arg + newPosition.x * y_lfo_other;
    if (y_phasemod_switch == 1) yLfoPhase += 0.01 * xLfo;
    
    float yLfo = yLfoAmp * oscillate(yLfoPhase, yLfoShape, 1, texCoordVarying);
    
    // Z oscillator with possible ring and phase modulation
    float zLfoAmp = z_lfo_amp;
    if (z_ringmod_switch == 1) zLfoAmp += 0.0025 * yLfo;
    
    float zLfoPhase = z_lfo_arg + z_lfo_other * distance(
        abs(newPosition.xy),
        vec2(xy_offset.x/2.0, xy_offset.y/2.0)
    );
    
    if (z_phasemod_switch == 1) zLfoPhase += yLfo;
    
    float zLfo = zLfoAmp * oscillate(zLfoPhase, zLfoShape, 2, texCoordVarying);
   
    // Apply Z modulation as a scaling factor
    newPosition.xy *= (1.0 - zLfo);
    
    // ===== Second oscillator pass =====
    
    // X oscillator (second pass) with possible modulations
    float xLfoAmpMod = x_lfo_amp;
    if (x_ringmod_switch == 1) xLfoAmpMod += 1000.0 * zLfo;
    
    float xLfoFreq = x_lfo_arg + newPosition.y * x_lfo_other;
    if (x_phasemod_switch == 1) xLfoFreq += 10.0 * zLfo;
    
    xLfo = xLfoAmpMod * oscillate(xLfoFreq, xLfoShape, 0, texCoordVarying);
    
    // Apply brightness and X oscillator displacement
    newPosition.x += xy.x * bright + xLfo;
    
    // Y oscillator (second pass) with possible modulations
    float yLfoAmpMod = y_lfo_amp;
    if (y_ringmod_switch == 1) yLfoAmpMod += xLfo;
    
    float yLfoFreq = y_lfo_arg + newPosition.x * y_lfo_other;
    if (y_phasemod_switch == 1) yLfoFreq += 0.01 * xLfo;
    
    yLfo = yLfoAmpMod * oscillate(yLfoFreq, yLfoShape, 1, texCoordVarying);
    
    // Apply brightness and Y oscillator displacement
    newPosition.y += xy.y * bright + yLfo;
    
    // Restore coordinates from centered position
    newPosition.x -= xy_offset.x;
    newPosition.y -= xy_offset.y;
    
    // Output final position
    gl_Position = newPosition;
}