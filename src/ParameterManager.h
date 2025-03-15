#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h" // Changed from ofXml.h to ofxXmlSettings.h

// Parameter lock system size
constexpr int PARAM_LOCK_SIZE = 240;
constexpr int PARAM_LOCK_COUNT = 16;

// Forward declarations
class MidiManager;

// Enum for the different mesh types
enum class MeshType {
    TriangleGrid,
    HorizontalLines,
    VerticalLines,
    TriangleWireframe
};

// Enum for LFO shapes
enum class LfoShape {
    Sine,
    Square,
    Triangle,
    Sawtooth
};

class ParameterManager {
public:
    ParameterManager();
    
    // Core methods
    void setup();
    void update();
    
    // Parameter Lock (P-Lock) system
    void startRecording();
    void stopRecording();
    void clearAllLocks();
    void updatePLocks();
    
    // Settings management
    bool loadSettings();
    bool saveSettings();
    void resetToDefaults();
    
    bool isDebugEnabled() const;
    void setDebugEnabled(bool enabled);
    
    bool isRecordingEnabled() const;
    
    // XML settings
    void loadFromXml(ofxXmlSettings& xml);
    
    void saveToXml(ofxXmlSettings& xml) const;
    
    // Performance mode settings
    bool isPerformanceModeEnabled() const;
    void setPerformanceModeEnabled(bool enabled);

    // Performance settings getters/setters
    int getPerformanceScale() const;
    void setPerformanceScale(int scale);

    int getNoiseUpdateInterval() const;
    void setNoiseUpdateInterval(int interval);

    bool isHighQualityEnabled() const;
    void setHighQualityEnabled(bool enabled);
    
    // Parameter access (getters/setters)
    
    // Video device settings
    std::string getVideoDevicePath() const;
    void setVideoDevicePath(const std::string& path);

    int getVideoDeviceID() const;
    void setVideoDeviceID(int id);

    std::string getVideoFormat() const;
    void setVideoFormat(const std::string& format);

    int getVideoWidth() const;
    void setVideoWidth(int width);

    int getVideoHeight() const;
    void setVideoHeight(int height);

    int getVideoFrameRate() const;
    void setVideoFrameRate(int fps);
    
    float getLumaKeyLevel() const;
    void setLumaKeyLevel(float value, bool recordable = true);
    
    float getXDisplace() const;
    void setXDisplace(float value, bool recordable = true);
    
    float getYDisplace() const;
    void setYDisplace(float value, bool recordable = true);
    
    float getZFrequency() const;
    void setZFrequency(float value, bool recordable = true);
    
    float getXFrequency() const;
    void setXFrequency(float value, bool recordable = true);
    
    float getYFrequency() const;
    void setYFrequency(float value, bool recordable = true);
    
    float getZoom() const;
    void setZoom(float value, bool recordable = true);
    
    int getScale() const;
    void setScale(int value, bool recordable = true);
    
    float getCenterX() const;
    void setCenterX(float value, bool recordable = true);
    
    float getCenterY() const;
    void setCenterY(float value, bool recordable = true);
    
    float getZLfoArg() const;
    void setZLfoArg(float value, bool recordable = true);
    
    float getZLfoAmp() const;
    void setZLfoAmp(float value, bool recordable = true);
    
    float getXLfoArg() const;
    void setXLfoArg(float value, bool recordable = true);
    
    float getXLfoAmp() const;
    void setXLfoAmp(float value, bool recordable = true);
    
    float getYLfoArg() const;
    void setYLfoArg(float value, bool recordable = true);
    
    float getYLfoAmp() const;
    void setYLfoAmp(float value, bool recordable = true);
    
    // Toggle states
    bool isWireframeEnabled() const;
    void setWireframeEnabled(bool enabled);
    
    bool isBrightEnabled() const;
    void setBrightEnabled(bool enabled);
    
    bool isInvertEnabled() const;
    void setInvertEnabled(bool enabled);
    
    bool isStrokeWeightEnabled() const;
    void setStrokeWeightEnabled(bool enabled);
    
    bool isBlackAndWhiteEnabled() const;
    void setBlackAndWhiteEnabled(bool enabled);
    
    bool isLumaKeyEnabled() const;
    void setLumaKeyEnabled(bool enabled);
    
    // LFO modulation
    LfoShape getYLfoShape() const;
    void setYLfoShape(LfoShape shape);
    
    LfoShape getXLfoShape() const;
    void setXLfoShape(LfoShape shape);
    
    LfoShape getZLfoShape() const;
    void setZLfoShape(LfoShape shape);
    
    bool isYRingModEnabled() const;
    void setYRingModEnabled(bool enabled);
    
    bool isXRingModEnabled() const;
    void setXRingModEnabled(bool enabled);
    
    bool isZRingModEnabled() const;
    void setZRingModEnabled(bool enabled);
    
    bool isYPhaseModEnabled() const;
    void setYPhaseModEnabled(bool enabled);
    
    bool isXPhaseModEnabled() const;
    void setXPhaseModEnabled(bool enabled);
    
    bool isZPhaseModEnabled() const;
    void setZPhaseModEnabled(bool enabled);
    
    // Transformation
    float getRotateX() const;
    void setRotateX(float value);
    
    float getRotateY() const;
    void setRotateY(float value);
    
    float getRotateZ() const;
    void setRotateZ(float value);
    
    float getGlobalXDisplace() const;
    void setGlobalXDisplace(float value);
    
    float getGlobalYDisplace() const;
    void setGlobalYDisplace(float value);
    
    // Mesh
    MeshType getMeshType() const;
    void setMeshType(MeshType type);
    
    float getStrokeWeight() const;
    void setStrokeWeight(float weight);
    
    struct MidiMapping {
        int controlNumber;     // CC number
        int channel;           // MIDI channel (0-15)
        std::string paramId;   // Parameter identifier (e.g., "luma_key_level")
        float minValue;        // Minimum parameter value
        float maxValue;        // Maximum parameter value
        bool bipolar;          // If true, centered at 0 (-1 to 1 range)
        
        // Constructor with defaults
        MidiMapping(int cc = 0, int ch = 0, const std::string& param = "",
                    float min = 0.0f, float max = 1.0f, bool isBipolar = false)
            : controlNumber(cc), channel(ch), paramId(param),
              minValue(min), maxValue(max), bipolar(isBipolar) {}
    };
    
    // Add a vector to store mappings
    std::vector<MidiMapping> midiMappings;

    // Add methods for handling mappings
    void addMidiMapping(const MidiMapping& mapping);
    void removeMidiMapping(int controlNumber, int channel = -1);
    MidiMapping* findMidiMapping(int controlNumber, int channel = -1);
    void clearAllMidiMappings();
    
    void generateMappingTemplate(const std::string& filename);
    
    // Get MIDI mappings
    const std::vector<MidiMapping>& getMidiMappings() const {
        return midiMappings;
    }
    
private:
    // XML settings
    ofxXmlSettings XML; // Changed from ofXml to ofxXmlSettings
    std::string settingsFile = "settings.xml";
    
    bool debugEnabled = false;
    bool performanceModeEnabled = false;
    int performanceScale = 50;  // Reduced scale when performance mode is on
    int noiseUpdateInterval = 4;  // Update noise every N frames
    bool highQualityEnabled = true;  // High quality rendering
    
    // Video device settings
    std::string videoDevicePath = "/dev/video0";
    int videoDeviceID = 0;
    std::string videoFormat = "yuyv422";
    int videoWidth = 640;
    int videoHeight = 480;
    int videoFrameRate = 30;
    
    // Parameter values
    float lumaKeyLevel = 0.0f;
    float xDisplace = 0.0f;
    float yDisplace = 0.0f;
    float zFrequency = 0.0f;
    float xFrequency = 0.0f;
    float yFrequency = 0.0f;
    float zoom = 0.0f;
    int scale = 100;
    float centerX = 0.0f;
    float centerY = 0.0f;
    float zLfoArg = 0.0f;
    float zLfoAmp = 0.0f;
    float xLfoArg = 0.0f;
    float xLfoAmp = 0.0f;
    float yLfoArg = 0.0f;
    float yLfoAmp = 0.0f;
    
    // Toggle states
    bool wireframeEnabled = false;
    bool brightEnabled = false;
    bool invertEnabled = false;
    bool strokeWeightEnabled = false;
    bool blackAndWhiteEnabled = false;
    bool lumaKeyEnabled = false;
    
    // LFO settings
    LfoShape yLfoShape = LfoShape::Sine;
    LfoShape xLfoShape = LfoShape::Sine;
    LfoShape zLfoShape = LfoShape::Sine;
    bool yRingModEnabled = false;
    bool xRingModEnabled = false;
    bool zRingModEnabled = false;
    bool yPhaseModEnabled = false;
    bool xPhaseModEnabled = false;
    bool zPhaseModEnabled = false;
    
    // Transformation
    float rotateX = 0.0f;
    float rotateY = 0.0f;
    float rotateZ = 0.0f;
    float globalXDisplace = 0.0f;
    float globalYDisplace = 0.0f;
    
    // Mesh
    MeshType meshType = MeshType::TriangleGrid;
    float strokeWeight = 1.0f;
    
    // P-Lock system
    bool recordingEnabled = false;
    int currentStep = 0;
    float pLockValues[PARAM_LOCK_COUNT][PARAM_LOCK_SIZE];
    float pLockSmoothedValues[PARAM_LOCK_COUNT];
    float pLockSmoothFactor = 0.5f;
    
    // Helper methods
    void recordParameter(int paramIndex, float value);
    
};
