#version 150
// For OpenGL 3.2+

in vec2 texCoordVarying;
out vec4 outputColor;

uniform sampler2D tex0;
uniform sampler2D x_noise_image;
uniform sampler2D y_noise_image;
uniform sampler2D z_noise_image;
uniform float luma_key_level;
uniform float invert_switch;
uniform float b_w_switch;
uniform int luma_switch;
uniform int bright_switch;
uniform vec2 xy;

void main() {
    // Sample the texture
    vec4 color = texture(tex0, texCoordVarying);
    
    // Calculate brightness using standard luminance weights
    float bright = dot(color.rgb, vec3(0.33, 0.5, 0.16));
    
    // Add protection against completely black textures
    bright = max(bright, 0.01);
    
    // Apply brightness inversion if enabled
    if (bright_switch == 1) {
        bright = 1.0 - bright;
    }
    
    // Apply black and white effect if enabled
    color = mix(color, vec4(vec3(bright), color.a), b_w_switch);
    
    // Apply inversion if enabled
    color.rgb = mix(color.rgb, 1.0 - color.rgb, invert_switch);
    
    // Apply luma key effects
    if (luma_switch == 0) {
        // Remove dark areas
        if (bright < luma_key_level) {
            color.a = 0.0;
        }
    } else {
        // Remove bright areas
        if (bright > luma_key_level) {
            color.a = 0.0;
        }
    }
    
    // Output the final color
    outputColor = color;
}