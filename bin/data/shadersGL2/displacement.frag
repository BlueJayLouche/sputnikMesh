OF_GLSL_SHADER_HEADER

varying vec2 texCoordVarying;

uniform sampler2D tex0;
uniform float luma_key_level;
uniform float invert_switch;
uniform float b_w_switch;
uniform int luma_switch;

void main() {
    // Sample the texture
    vec4 color = texture2D(tex0, texCoordVarying);
    
    // Calculate brightness using standard luminance weights
    float bright = dot(color.rgb, vec3(0.33, 0.5, 0.16));
    
    // Apply black and white effect if enabled
    vec4 bw = vec4(vec3(bright), color.a);
    color = mix(color, bw, b_w_switch);
    
    // Apply inversion if enabled
    vec3 inverted = 1.0 - color.rgb;
    color.rgb = mix(color.rgb, inverted, invert_switch);
    
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
    gl_FragColor = color;
}
