OF_GLSL_SHADER_HEADER

in vec2 texCoordVarying;
out vec4 outputColor;

uniform sampler2D tex0;
uniform float luma_key_level;
uniform float invert_switch;
uniform float b_w_switch;
uniform int luma_switch;

void main() {
    // Sample the texture
    vec4 color = texture(tex0, texCoordVarying);
    
    // Display texture coordinates as colors in the middle section
    if(gl_FragCoord.x >= 100 && gl_FragCoord.x <= 540) {
        // Show texture coordinates as colors
        outputColor = vec4(texCoordVarying.x, texCoordVarying.y, 0.5, 1.0);
        return;
    }
    
    // Left portion of screen: straight camera texture 
    if(gl_FragCoord.x < 100) {
        outputColor = color;
        return;
    }
    
    // Right portion: grid pattern to check shader execution
    if(gl_FragCoord.x > 540) {
        float gridSize = 20.0;
        vec2 grid = floor(gl_FragCoord.xy / gridSize);
        float checker = mod(grid.x + grid.y, 2.0);
        outputColor = vec4(checker, 0.5, 1.0-checker, 1.0);
        return;
    }
    
    // Normal shader processing
    // Calculate brightness using standard luminance weights
    float bright = dot(color.rgb, vec3(0.33, 0.5, 0.16));
    
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