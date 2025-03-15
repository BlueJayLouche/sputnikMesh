#pragma once

#include "ofMain.h"

/**
 * @class TextureHelper
 * @brief Helper for handling texture access across OpenGL versions
 *
 * This class provides utility methods for working with textures
 * in a cross-platform compatible way. It handles the differences
 * between modern OpenGL and ES2 texture functions.
 */
class TextureHelper {
public:
    /**
     * Creates a compatibility shader fragment for texture sampling
     * to handle differences between texture2D() and texture() functions
     *
     * @return A shader string with the appropriate texture function
     */
    static std::string getCompatibilityHeader() {
        if (ofIsGLProgrammableRenderer()) {
            // Modern OpenGL with GLSL 1.5+
            return R"(
                #version 150
                #define SAMPLER_FN texture
            )";
        } else {
            // OpenGL ES2 or old OpenGL
            return R"(
                #version 120
                #define SAMPLER_FN texture2D
            )";
        }
    }
    
    /**
     * Get appropriate texture precision definition for fragment shaders
     *
     * @return A shader string with the appropriate precision declaration
     */
    static std::string getFragmentPrecision() {
        if (ofIsGLProgrammableRenderer()) {
            // Modern OpenGL uses precision qualifiers optionally
            return "";
        } else {
            // OpenGL ES requires precision qualifiers
            return "precision highp float;\n";
        }
    }
    
    /**
     * Get appropriate version string for shader
     */
    static std::string getVersionString() {
        if (ofIsGLProgrammableRenderer()) {
            return "#version 150\n";
        } else {
            #ifdef TARGET_OPENGLES
            return "#version 100\nprecision highp float;\n";
            #else
            return "#version 120\n";
            #endif
        }
    }
    
    /**
     * Replace texture2D() calls with texture() for modern OpenGL
     *
     * @param shaderSource Original shader source code
     * @return Modified shader source
     */
    static std::string fixTextureFunction(const std::string& shaderSource) {
        if (ofIsGLProgrammableRenderer()) {
            std::string modifiedSource = shaderSource;
            ofStringReplace(modifiedSource, "texture2D(", "texture(");
            return modifiedSource;
        } else {
            return shaderSource;
        }
    }
};
