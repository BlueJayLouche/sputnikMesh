#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "MidiManager.h"
#include "ParameterManager.h"
#include "MeshGenerator.h"
#include "ShaderLoader.h"
#include "ofxXmlSettings.h"
#include "AudioReactivityManager.h"
#include "ofxFft.h"
#include "V4L2Helper.h"

class ofApp : public ofBaseApp {
    
public:
    // Core application methods
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    
    // Camera handling
    void setupCamera(int width, int height);
    void updateCamera();
    
    // New video device functions
    void listVideoDevices();
    void drawVideoDevicesUI();
    bool selectVideoDevice(int deviceIndex);
    bool selectVideoDevice(const std::string& devicePath);
    bool setVideoFormat(const std::string& format, int width, int height);
    void cycleVideoFormat();
    
    // FBO management
    void allocateFbos();
    ofFbo cameraFbo;  // For converting camera input to RGBA
    
    // Noise generation
    ofImage generatePerlinNoise(float theta, float resolution, ofImage& noiseImage);
private:
    void drawDebugInfo();
    
    // Configuration
    int width = 640;
    int height = 480;
    
    // Managers for subsystems
    std::unique_ptr<MidiManager> midiManager;
    std::unique_ptr<ParameterManager> paramManager;
    std::unique_ptr<MeshGenerator> meshGenerator;
    
    // Camera input
    ofVideoGrabber camera;
    bool hdmiAspectRatioEnabled = false;
    
    // New video device data
    std::vector<V4L2Helper::VideoDevice> videoDevices;
    std::vector<V4L2Helper::VideoFormat> videoFormats;
    std::vector<V4L2Helper::Resolution> videoResolutions;
    int currentFormatIndex = 0;
    int currentResolutionIndex = 0;
    
    // FBOs
    ofFbo mainFbo;
    ofFbo aspectFixFbo;
    
    // Shader
    ofShader shaderDisplace;
    
    // Noise textures
    ofImage xNoiseImage;
    ofImage yNoiseImage;
    ofImage zNoiseImage;
    
    // LFO state
    float xLfoArg = 0.0f;
    float yLfoArg = 0.0f;
    float zLfoArg = 0.0f;
    
    // Key-controlled parameters (temporary, will move to ParameterManager)
    struct KeyControls {
        float lumaKey = 0.0f;
        float xDisplace = 0.0f;
        float yDisplace = 0.0f;
        float zFrequency = 0.0f;
        float xFrequency = 0.0f;
        float yFrequency = 0.0f;
        float zoom = 0.0f;
        float zLfoArg = 0.0f;
        float xLfoArg = 0.0f;
        float yLfoArg = 0.0f;
        float zLfoAmp = 0.0f;
        float xLfoAmp = 0.0f;
        float yLfoAmp = 0.0f;
        float centerX = 0.0f;
        float centerY = 0.0f;
    } keyControls;
    
    bool showMidiMappings = true; // Whether to show MIDI mappings in debug view
    
    float performanceMetrics[60]; // Store last 60 frame times
    int metricIndex = 0;
    
    std::unique_ptr<AudioReactivityManager> audioManager;
    void drawAudioVisualization();
    void drawAudioDevicesUI();
    
    void setupAudioReactivity();
};
