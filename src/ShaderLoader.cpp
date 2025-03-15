#pragma once

#include "ofMain.h"

/**
 * @class ShaderLoader
 * @brief Cross-platform shader loading helper
 *
 * This class handles loading the appropriate shader version based on
 * the current platform and OpenGL context. It automatically selects
 * between desktop and ES2 shader versions.
 */
class ShaderLoader {
public:
    /**
     * Load the appropriate shader version for the current platform
     *
     * @param shader The shader object to load into
     * @param shaderName Base name of the shader (without extension)
     * @return True if loading was successful, false otherwise
     */
    static bool load(ofShader& shader, const std::string& shaderName) {
        bool success = false;
        
        // Check if we're using programmable renderer (OpenGL 3.2+)
        if (ofIsGLProgrammableRenderer()) {
            ofLogNotice("ShaderLoader") << "Loading modern shader: " << shaderName;
            success = shader.load(shaderName);
        }
        // If not, we're using a fixed function pipeline (OpenGL 2.1 or ES2)
        else {
            // Check if we're on a mobile/embedded platform
            #ifdef TARGET_OPENGLES
                ofLogNotice("ShaderLoader") << "Loading ES2 shader: " << shaderName + "ES2";
                success = shader.load(shaderName + "ES2");
            #else
                ofLogNotice("ShaderLoader") << "Loading desktop shader: " << shaderName;
                success = shader.load(shaderName);
            #endif
        }
        
        if (!success) {
            ofLogError("ShaderLoader") << "Failed to load shader: " << shaderName;
        }
        
        return success;
    }
    
    /**
     * Load the appropriate shader version from explicit vertex and fragment paths
     *
     * @param shader The shader object to load into
     * @param vertPath Path to the vertex shader
     * @param fragPath Path to the fragment shader
     * @return True if loading was successful, false otherwise
     */
    static bool loadFromPaths(ofShader& shader, const std::string& vertPath, const std::string& fragPath) {
        bool success = shader.load(vertPath, fragPath);
        
        if (!success) {
            ofLogError("ShaderLoader") << "Failed to load shader from paths: " << vertPath << ", " << fragPath;
        }
        
        return success;
    }
    
    /**
     * Get recommended shader directory based on platform
     * @return Path to the recommended shader directory
     */
    static std::string getShaderDirectory() {
        // For mobile platforms
        #ifdef TARGET_OPENGLES
            return "shadersES2/";
        #else
            // For desktop platforms
            return "shaders/";
        #endif
    }
};
