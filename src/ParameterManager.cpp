#include "ParameterManager.h"

ParameterManager::ParameterManager() {
    // Initialize P-Lock arrays with zeros
    for (int i = 0; i < PARAM_LOCK_COUNT; i++) {
        pLockSmoothedValues[i] = 0.0f;
        for (int j = 0; j < PARAM_LOCK_SIZE; j++) {
            pLockValues[i][j] = 0.0f;
        }
    }
    
    // Initialize performance mode settings with default values
    performanceModeEnabled = false;
    performanceScale = 50;
    noiseUpdateInterval = 4;
    highQualityEnabled = true;
}

void ParameterManager::setup() {
    // Try to load settings from XML, fall back to defaults if not found
    if (!loadSettings()) {
        resetToDefaults();
    }
}

void ParameterManager::update() {
    // Update P-Lock system
    updatePLocks();
}

bool ParameterManager::isDebugEnabled() const {
    return debugEnabled;
}

void ParameterManager::setDebugEnabled(bool enabled) {
    debugEnabled = enabled;
}

bool ParameterManager::isRecordingEnabled() const {
    return recordingEnabled;
}

void ParameterManager::startRecording() {
    recordingEnabled = true;
    
    // Copy current P-Lock values to all steps
    for (int i = 0; i < PARAM_LOCK_COUNT; i++) {
        float currentValue = pLockValues[i][currentStep];
        for (int j = 0; j < PARAM_LOCK_SIZE; j++) {
            pLockValues[i][j] = currentValue;
        }
    }
}

void ParameterManager::stopRecording() {
    recordingEnabled = false;
}

void ParameterManager::clearAllLocks() {
    for (int i = 0; i < PARAM_LOCK_COUNT; i++) {
        pLockSmoothedValues[i] = 0.0f;
        for (int j = 0; j < PARAM_LOCK_SIZE; j++) {
            pLockValues[i][j] = 0.0f;
        }
    }
}

void ParameterManager::updatePLocks() {
    // Apply smoothing to all parameters
    for (int i = 0; i < PARAM_LOCK_COUNT; i++) {
        // Apply smoothing formula: smoothed = current * (1-factor) + previous * factor
        pLockSmoothedValues[i] = pLockValues[i][currentStep] * (1.0f - pLockSmoothFactor)
                               + pLockSmoothedValues[i] * pLockSmoothFactor;
        
        // Eliminate very small values to prevent jitter
        if (abs(pLockSmoothedValues[i]) < 0.01) {
            pLockSmoothedValues[i] = 0.0f;
        }
    }
    
    // Increment step if recording is enabled
    if (recordingEnabled) {
        currentStep = (currentStep + 1) % PARAM_LOCK_SIZE;
    }
}

bool ParameterManager::loadSettings() {
    ofxXmlSettings xml;
    if (xml.loadFile(ofToDataPath(settingsFile))) {
        loadFromXml(xml);
        return true;
    }
    return false;
}

bool ParameterManager::saveSettings() {
    ofxXmlSettings xml;
    saveToXml(xml);
    return xml.saveFile(ofToDataPath(settingsFile));
}

void ParameterManager::resetToDefaults() {
    // Reset basic parameters
    lumaKeyLevel = 0.0f;
    xDisplace = 0.0f;
    yDisplace = 0.0f;
    zFrequency = 0.03f;
    xFrequency = 0.015f;
    yFrequency = 0.02f;
    zoom = 0.0f;
    scale = 100;
    centerX = 0.0f;
    centerY = 0.0f;
    zLfoArg = 0.0f;
    zLfoAmp = 0.0f;
    xLfoArg = 0.0f;
    xLfoAmp = 0.0f;
    yLfoArg = 0.0f;
    yLfoAmp = 0.0f;
    
    // Reset toggle states
    wireframeEnabled = false;
    brightEnabled = false;
    invertEnabled = false;
    strokeWeightEnabled = false;
    blackAndWhiteEnabled = false;
    lumaKeyEnabled = false;
    
    // Reset LFO settings
    yLfoShape = LfoShape::Sine;
    xLfoShape = LfoShape::Sine;
    zLfoShape = LfoShape::Sine;
    yRingModEnabled = false;
    xRingModEnabled = false;
    zRingModEnabled = false;
    yPhaseModEnabled = false;
    xPhaseModEnabled = false;
    zPhaseModEnabled = false;
    
    // Reset transformation
    rotateX = 0.0f;
    rotateY = 0.0f;
    rotateZ = 0.0f;
    globalXDisplace = 0.0f;
    globalYDisplace = 0.0f;
    
    // Reset mesh
    meshType = MeshType::TriangleGrid;
    strokeWeight = 1.0f;
    
    // Clear P-Lock system
    clearAllLocks();
}

void ParameterManager::recordParameter(int paramIndex, float value) {
    // Check if recording is enabled and the parameter index is valid
    if (recordingEnabled && paramIndex >= 0 && paramIndex < PARAM_LOCK_COUNT) {
        // Check if we have a valid step index
        if (currentStep >= 0 && currentStep < PARAM_LOCK_SIZE) {
            pLockValues[paramIndex][currentStep] = value;
        } else {
            ofLogError("ParameterManager") << "Invalid P-Lock step index: " << currentStep;
        }
    }
}

void ParameterManager::loadFromXml(ofxXmlSettings& xml) {
    // Load basic parameters with default values
    
    debugEnabled = xml.getValue("toggles:debug", false);
    performanceModeEnabled = xml.getValue("performance:enabled", false);
    performanceScale = xml.getValue("performance:scale", 50);
    noiseUpdateInterval = xml.getValue("performance:noiseUpdateInterval", 4);
    highQualityEnabled = xml.getValue("performance:highQuality", true);
    
    // Load video settings
    videoDevicePath = xml.getValue("video:devicePath", "/dev/video0");
    videoDeviceID = xml.getValue("video:deviceID", 0);
    videoFormat = xml.getValue("video:format", "yuyv422");
    videoWidth = xml.getValue("video:width", 640);
    videoHeight = xml.getValue("video:height", 480);
    videoFrameRate = xml.getValue("video:frameRate", 30);
    
    lumaKeyLevel = xml.getValue("parameters:lumaKeyLevel", 0.0f);
    xDisplace = xml.getValue("parameters:xDisplace", 0.0f);
    yDisplace = xml.getValue("parameters:yDisplace", 0.0f);
    zFrequency = xml.getValue("parameters:zFrequency", 0.03f);
    xFrequency = xml.getValue("parameters:xFrequency", 0.015f);
    yFrequency = xml.getValue("parameters:yFrequency", 0.02f);
    zoom = xml.getValue("parameters:zoom", 0.0f);
    scale = xml.getValue("parameters:scale", 100);
    centerX = xml.getValue("parameters:centerX", 0.0f);
    centerY = xml.getValue("parameters:centerY", 0.0f);
    zLfoArg = xml.getValue("parameters:zLfoArg", 0.0f);
    zLfoAmp = xml.getValue("parameters:zLfoAmp", 0.0f);
    xLfoArg = xml.getValue("parameters:xLfoArg", 0.0f);
    xLfoAmp = xml.getValue("parameters:xLfoAmp", 0.0f);
    yLfoArg = xml.getValue("parameters:yLfoArg", 0.0f);
    yLfoAmp = xml.getValue("parameters:yLfoAmp", 0.0f);
    
    // Load toggle states
    wireframeEnabled = xml.getValue("toggles:wireframe", false);
    brightEnabled = xml.getValue("toggles:bright", false);
    invertEnabled = xml.getValue("toggles:invert", false);
    strokeWeightEnabled = xml.getValue("toggles:strokeWeight", false);
    blackAndWhiteEnabled = xml.getValue("toggles:blackAndWhite", false);
    lumaKeyEnabled = xml.getValue("toggles:lumaKey", false);
    
    // Load LFO settings
    int tempShape = xml.getValue("lfo:yShape", 0);
    yLfoShape = static_cast<LfoShape>(tempShape);
    
    tempShape = xml.getValue("lfo:xShape", 0);
    xLfoShape = static_cast<LfoShape>(tempShape);
    
    tempShape = xml.getValue("lfo:zShape", 0);
    zLfoShape = static_cast<LfoShape>(tempShape);
    
    yRingModEnabled = xml.getValue("lfo:yRingMod", false);
    xRingModEnabled = xml.getValue("lfo:xRingMod", false);
    zRingModEnabled = xml.getValue("lfo:zRingMod", false);
    yPhaseModEnabled = xml.getValue("lfo:yPhaseMod", false);
    xPhaseModEnabled = xml.getValue("lfo:xPhaseMod", false);
    zPhaseModEnabled = xml.getValue("lfo:zPhaseMod", false);
    
    // Load transformation
    rotateX = xml.getValue("transform:rotateX", 0.0f);
    rotateY = xml.getValue("transform:rotateY", 0.0f);
    rotateZ = xml.getValue("transform:rotateZ", 0.0f);
    globalXDisplace = xml.getValue("transform:globalXDisplace", 0.0f);
    globalYDisplace = xml.getValue("transform:globalYDisplace", 0.0f);
    
    // Load mesh
    int tempMeshType = xml.getValue("mesh:type", 0);
    meshType = static_cast<MeshType>(tempMeshType);
    strokeWeight = xml.getValue("mesh:strokeWeight", 1.0f);
    
    // Load P-Lock data if available
    xml.pushTag("plocks");
    int numPlocks = xml.getNumTags("plock");
    
    for (int i = 0; i < numPlocks && i < PARAM_LOCK_COUNT; i++) {
        xml.pushTag("plock", i);
        
        int id = xml.getValue("id", i);
        string valuesStr = xml.getValue("values", "");
        
        if (!valuesStr.empty()) {
            vector<string> values = ofSplitString(valuesStr, ",");
            
            for (int j = 0; j < min(static_cast<int>(values.size()), PARAM_LOCK_SIZE); j++) {
                pLockValues[id][j] = ofToFloat(values[j]);
            }
        }
        
        xml.popTag(); // pop plock
    }
    
    xml.popTag(); // pop plocks
    
    // Clear existing mappings before loading
    midiMappings.clear();
    
    // Load MIDI mappings if they exist
    if (xml.tagExists("midiMappings")) {
        xml.pushTag("midiMappings");
        
        int numMappings = xml.getNumTags("mapping");
        for (int i = 0; i < numMappings; i++) {
            xml.pushTag("mapping", i);
            
            MidiMapping mapping;
            mapping.controlNumber = xml.getValue("controlNumber", 0);
            mapping.channel = xml.getValue("channel", 0);
            mapping.paramId = xml.getValue("paramId", "");
            mapping.minValue = xml.getValue("minValue", 0.0f);
            mapping.maxValue = xml.getValue("maxValue", 1.0f);
            mapping.bipolar = xml.getValue("bipolar", false);
            
            midiMappings.push_back(mapping);
            
            xml.popTag(); // pop mapping
        }
        
        xml.popTag(); // pop midiMappings
    }
}

void ParameterManager::saveToXml(ofxXmlSettings& xml) const {
    // ===================================================================
    // Check if major tags already exist, remove them to avoid duplications
    // ===================================================================
    if (xml.tagExists("parameters")) {
        xml.removeTag("parameters");
    }
    
    if (xml.tagExists("toggles")) {
        xml.removeTag("toggles");
    }
    
    if (xml.tagExists("lfo")) {
        xml.removeTag("lfo");
    }
    
    if (xml.tagExists("transform")) {
        xml.removeTag("transform");
    }
    
    if (xml.tagExists("mesh")) {
        xml.removeTag("mesh");
    }
    
    if (xml.tagExists("plocks")) {
        xml.removeTag("plocks");
    }
    
    if (xml.tagExists("midiMappings")) {
        xml.removeTag("midiMappings");
    }
    
    if (xml.tagExists("video")) {
        xml.removeTag("video");
    }
    
    // ===================================================================
    // Create XML structure for basic parameters
    // ===================================================================
    
    // Debug toggle
    xml.setValue("toggles:debug", debugEnabled);
    xml.setValue("performance:enabled", performanceModeEnabled);
    xml.setValue("performance:scale", performanceScale);
    xml.setValue("performance:noiseUpdateInterval", noiseUpdateInterval);
    xml.setValue("performance:highQuality", highQualityEnabled);
    
    // Basic parameters
    xml.setValue("parameters:lumaKeyLevel", lumaKeyLevel);
    xml.setValue("parameters:xDisplace", xDisplace);
    xml.setValue("parameters:yDisplace", yDisplace);
    xml.setValue("parameters:zFrequency", zFrequency);
    xml.setValue("parameters:xFrequency", xFrequency);
    xml.setValue("parameters:yFrequency", yFrequency);
    xml.setValue("parameters:zoom", zoom);
    xml.setValue("parameters:scale", scale);
    xml.setValue("parameters:centerX", centerX);
    xml.setValue("parameters:centerY", centerY);
    xml.setValue("parameters:zLfoArg", zLfoArg);
    xml.setValue("parameters:zLfoAmp", zLfoAmp);
    xml.setValue("parameters:xLfoArg", xLfoArg);
    xml.setValue("parameters:xLfoAmp", xLfoAmp);
    xml.setValue("parameters:yLfoArg", yLfoArg);
    xml.setValue("parameters:yLfoAmp", yLfoAmp);
    
    // Toggle states
    xml.setValue("toggles:wireframe", wireframeEnabled);
    xml.setValue("toggles:bright", brightEnabled);
    xml.setValue("toggles:invert", invertEnabled);
    xml.setValue("toggles:strokeWeight", strokeWeightEnabled);
    xml.setValue("toggles:blackAndWhite", blackAndWhiteEnabled);
    xml.setValue("toggles:lumaKey", lumaKeyEnabled);
    
    // LFO settings
    xml.setValue("lfo:yShape", static_cast<int>(yLfoShape));
    xml.setValue("lfo:xShape", static_cast<int>(xLfoShape));
    xml.setValue("lfo:zShape", static_cast<int>(zLfoShape));
    xml.setValue("lfo:yRingMod", yRingModEnabled);
    xml.setValue("lfo:xRingMod", xRingModEnabled);
    xml.setValue("lfo:zRingMod", zRingModEnabled);
    xml.setValue("lfo:yPhaseMod", yPhaseModEnabled);
    xml.setValue("lfo:xPhaseMod", xPhaseModEnabled);
    xml.setValue("lfo:zPhaseMod", zPhaseModEnabled);
    
    // Transformation
    xml.setValue("transform:rotateX", rotateX);
    xml.setValue("transform:rotateY", rotateY);
    xml.setValue("transform:rotateZ", rotateZ);
    xml.setValue("transform:globalXDisplace", globalXDisplace);
    xml.setValue("transform:globalYDisplace", globalYDisplace);
    
    // Mesh
    xml.setValue("mesh:type", static_cast<int>(meshType));
    xml.setValue("mesh:strokeWeight", strokeWeight);
    
    // Video settings
    xml.setValue("video:devicePath", videoDevicePath);
    xml.setValue("video:deviceID", videoDeviceID);
    xml.setValue("video:format", videoFormat);
    xml.setValue("video:width", videoWidth);
    xml.setValue("video:height", videoHeight);
    xml.setValue("video:frameRate", videoFrameRate);
    
    // Save P-Lock data
    xml.addTag("plocks");
    xml.pushTag("plocks");
    
    for (int i = 0; i < PARAM_LOCK_COUNT; i++) {
        xml.addTag("plock");
        xml.pushTag("plock", i);
        
        xml.setValue("id", i);
        
        std::string valuesStr = "";
        for (int j = 0; j < PARAM_LOCK_SIZE; j++) {
            valuesStr += ofToString(pLockValues[i][j]);
            if (j < PARAM_LOCK_SIZE - 1) {
                valuesStr += ",";
            }
        }
        
        xml.setValue("values", valuesStr);
        xml.popTag(); // pop plock
    }
    
    xml.popTag(); // pop plocks
    
    // Save MIDI mappings
    xml.addTag("midiMappings");
    xml.pushTag("midiMappings");
    
    for (size_t i = 0; i < midiMappings.size(); i++) {
        const auto& mapping = midiMappings[i];
        
        xml.addTag("mapping");
        xml.pushTag("mapping", i);
        
        xml.setValue("controlNumber", mapping.controlNumber);
        xml.setValue("channel", mapping.channel);
        xml.setValue("paramId", mapping.paramId);
        xml.setValue("minValue", mapping.minValue);
        xml.setValue("maxValue", mapping.maxValue);
        xml.setValue("bipolar", mapping.bipolar);
        
        xml.popTag(); // pop mapping
    }
    
    xml.popTag(); // pop midiMappings
}

// Getter and setter implementations for all parameters


std::string ParameterManager::getVideoDevicePath() const {
    return videoDevicePath;
}

void ParameterManager::setVideoDevicePath(const std::string& path) {
    videoDevicePath = path;
}

int ParameterManager::getVideoDeviceID() const {
    return videoDeviceID;
}

void ParameterManager::setVideoDeviceID(int id) {
    videoDeviceID = id;
}

std::string ParameterManager::getVideoFormat() const {
    return videoFormat;
}

void ParameterManager::setVideoFormat(const std::string& format) {
    videoFormat = format;
}

int ParameterManager::getVideoWidth() const {
    return videoWidth;
}

void ParameterManager::setVideoWidth(int width) {
    videoWidth = width;
}

int ParameterManager::getVideoHeight() const {
    return videoHeight;
}

void ParameterManager::setVideoHeight(int height) {
    videoHeight = height;
}

int ParameterManager::getVideoFrameRate() const {
    return videoFrameRate;
}

void ParameterManager::setVideoFrameRate(int fps) {
    videoFrameRate = fps;
}

float ParameterManager::getLumaKeyLevel() const {
    return lumaKeyLevel + pLockSmoothedValues[0];
}

void ParameterManager::setLumaKeyLevel(float value, bool recordable) {
    lumaKeyLevel = value;
    if (recordable) {
        recordParameter(0, value);
    }
}

float ParameterManager::getXDisplace() const {
    return xDisplace + pLockSmoothedValues[1];
}

void ParameterManager::setXDisplace(float value, bool recordable) {
    xDisplace = value;
    if (recordable) {
        recordParameter(1, value);
    }
}

float ParameterManager::getYDisplace() const {
    return yDisplace + pLockSmoothedValues[2];
}

void ParameterManager::setYDisplace(float value, bool recordable) {
    yDisplace = value;
    if (recordable) {
        recordParameter(2, value);
    }
}

float ParameterManager::getZFrequency() const {
    return zFrequency + pLockSmoothedValues[3];
}

void ParameterManager::setZFrequency(float value, bool recordable) {
    zFrequency = value;
    if (recordable) {
        recordParameter(3, value);
    }
}

float ParameterManager::getXFrequency() const {
    return xFrequency + pLockSmoothedValues[4];
}

void ParameterManager::setXFrequency(float value, bool recordable) {
    xFrequency = value;
    if (recordable) {
        recordParameter(4, value);
    }
}

float ParameterManager::getYFrequency() const {
    return yFrequency + pLockSmoothedValues[5];
}

void ParameterManager::setYFrequency(float value, bool recordable) {
    yFrequency = value;
    if (recordable) {
        recordParameter(5, value);
    }
}

float ParameterManager::getZoom() const {
    return zoom + pLockSmoothedValues[6];
}

void ParameterManager::setZoom(float value, bool recordable) {
    zoom = value;
    if (recordable) {
        recordParameter(6, value);
    }
}

int ParameterManager::getScale() const {
    return scale;
}

void ParameterManager::setScale(int value, bool recordable) {
    scale = value;
    if (recordable) {
        // Note: For integer parameters, we'll convert to float for P-Lock system
        recordParameter(7, static_cast<float>(value));
    }
}

float ParameterManager::getCenterX() const {
    return centerX + pLockSmoothedValues[8];
}

void ParameterManager::setCenterX(float value, bool recordable) {
    centerX = value;
    if (recordable) {
        recordParameter(8, value);
    }
}

float ParameterManager::getCenterY() const {
    return centerY + pLockSmoothedValues[9];
}

void ParameterManager::setCenterY(float value, bool recordable) {
    centerY = value;
    if (recordable) {
        recordParameter(9, value);
    }
}

float ParameterManager::getZLfoArg() const {
    return zLfoArg + pLockSmoothedValues[10];
}

void ParameterManager::setZLfoArg(float value, bool recordable) {
    zLfoArg = value;
    if (recordable) {
        recordParameter(10, value);
    }
}

float ParameterManager::getZLfoAmp() const {
    return zLfoAmp + pLockSmoothedValues[11];
}

void ParameterManager::setZLfoAmp(float value, bool recordable) {
    zLfoAmp = value;
    if (recordable) {
        recordParameter(11, value);
    }
}

float ParameterManager::getXLfoArg() const {
    return xLfoArg + pLockSmoothedValues[12];
}

void ParameterManager::setXLfoArg(float value, bool recordable) {
    xLfoArg = value;
    if (recordable) {
        recordParameter(12, value);
    }
}

float ParameterManager::getXLfoAmp() const {
    return xLfoAmp + pLockSmoothedValues[13];
}

void ParameterManager::setXLfoAmp(float value, bool recordable) {
    xLfoAmp = value;
    if (recordable) {
        recordParameter(13, value);
    }
}

float ParameterManager::getYLfoArg() const {
    return yLfoArg + pLockSmoothedValues[14];
}

void ParameterManager::setYLfoArg(float value, bool recordable) {
    yLfoArg = value;
    if (recordable) {
        recordParameter(14, value);
    }
}

float ParameterManager::getYLfoAmp() const {
    return yLfoAmp + pLockSmoothedValues[15];
}

void ParameterManager::setYLfoAmp(float value, bool recordable) {
    yLfoAmp = value;
    if (recordable) {
        recordParameter(15, value);
    }
}

// Toggle state getters and setters
bool ParameterManager::isWireframeEnabled() const {
    return wireframeEnabled;
}

void ParameterManager::setWireframeEnabled(bool enabled) {
    wireframeEnabled = enabled;
}

bool ParameterManager::isBrightEnabled() const {
    return brightEnabled;
}

void ParameterManager::setBrightEnabled(bool enabled) {
    brightEnabled = enabled;
}

bool ParameterManager::isInvertEnabled() const {
    return invertEnabled;
}

void ParameterManager::setInvertEnabled(bool enabled) {
    invertEnabled = enabled;
}

bool ParameterManager::isStrokeWeightEnabled() const {
    return strokeWeightEnabled;
}

void ParameterManager::setStrokeWeightEnabled(bool enabled) {
    strokeWeightEnabled = enabled;
}

bool ParameterManager::isBlackAndWhiteEnabled() const {
    return blackAndWhiteEnabled;
}

void ParameterManager::setBlackAndWhiteEnabled(bool enabled) {
    blackAndWhiteEnabled = enabled;
}

bool ParameterManager::isLumaKeyEnabled() const {
    return lumaKeyEnabled;
}

void ParameterManager::setLumaKeyEnabled(bool enabled) {
    lumaKeyEnabled = enabled;
}

// LFO modulation getters and setters
LfoShape ParameterManager::getYLfoShape() const {
    return yLfoShape;
}

void ParameterManager::setYLfoShape(LfoShape shape) {
    yLfoShape = shape;
}

LfoShape ParameterManager::getXLfoShape() const {
    return xLfoShape;
}

void ParameterManager::setXLfoShape(LfoShape shape) {
    xLfoShape = shape;
}

LfoShape ParameterManager::getZLfoShape() const {
    return zLfoShape;
}

void ParameterManager::setZLfoShape(LfoShape shape) {
    zLfoShape = shape;
}

bool ParameterManager::isYRingModEnabled() const {
    return yRingModEnabled;
}

void ParameterManager::setYRingModEnabled(bool enabled) {
    yRingModEnabled = enabled;
}

bool ParameterManager::isXRingModEnabled() const {
    return xRingModEnabled;
}

void ParameterManager::setXRingModEnabled(bool enabled) {
    xRingModEnabled = enabled;
}

bool ParameterManager::isZRingModEnabled() const {
    return zRingModEnabled;
}

void ParameterManager::setZRingModEnabled(bool enabled) {
    zRingModEnabled = enabled;
}

bool ParameterManager::isYPhaseModEnabled() const {
    return yPhaseModEnabled;
}

void ParameterManager::setYPhaseModEnabled(bool enabled) {
    yPhaseModEnabled = enabled;
}

bool ParameterManager::isXPhaseModEnabled() const {
    return xPhaseModEnabled;
}

void ParameterManager::setXPhaseModEnabled(bool enabled) {
    xPhaseModEnabled = enabled;
}

bool ParameterManager::isZPhaseModEnabled() const {
    return zPhaseModEnabled;
}

void ParameterManager::setZPhaseModEnabled(bool enabled) {
    zPhaseModEnabled = enabled;
}

// Transformation getters and setters
float ParameterManager::getRotateX() const {
    return rotateX;
}

void ParameterManager::setRotateX(float value) {
    rotateX = value;
}

float ParameterManager::getRotateY() const {
    return rotateY;
}

void ParameterManager::setRotateY(float value) {
    rotateY = value;
}

float ParameterManager::getRotateZ() const {
    return rotateZ;
}

void ParameterManager::setRotateZ(float value) {
    rotateZ = value;
}

float ParameterManager::getGlobalXDisplace() const {
    return globalXDisplace;
}

void ParameterManager::setGlobalXDisplace(float value) {
    globalXDisplace = value;
}

float ParameterManager::getGlobalYDisplace() const {
    return globalYDisplace;
}

void ParameterManager::setGlobalYDisplace(float value) {
    globalYDisplace = value;
}

// Mesh getters and setters
MeshType ParameterManager::getMeshType() const {
    return meshType;
}

void ParameterManager::setMeshType(MeshType type) {
    meshType = type;
}

float ParameterManager::getStrokeWeight() const {
    return strokeWeight;
}

void ParameterManager::setStrokeWeight(float weight) {
    strokeWeight = weight;
}

//PERFORMANCE MODE----------------
bool ParameterManager::isPerformanceModeEnabled() const {
    return performanceModeEnabled;
}

void ParameterManager::setPerformanceModeEnabled(bool enabled) {
    performanceModeEnabled = enabled;
}

int ParameterManager::getPerformanceScale() const {
    return performanceScale;
}

void ParameterManager::setPerformanceScale(int scale) {
    performanceScale = scale;
}

int ParameterManager::getNoiseUpdateInterval() const {
    return noiseUpdateInterval;
}

void ParameterManager::setNoiseUpdateInterval(int interval) {
    noiseUpdateInterval = interval;
}

bool ParameterManager::isHighQualityEnabled() const {
    return highQualityEnabled;
}

void ParameterManager::setHighQualityEnabled(bool enabled) {
    highQualityEnabled = enabled;
}

//---------------------------------
void ParameterManager::addMidiMapping(const ParameterManager::MidiMapping& mapping) {
    // Check if mapping already exists
    auto existing = findMidiMapping(mapping.controlNumber, mapping.channel);
    if (existing) {
        // Update existing mapping
        *existing = mapping;
    } else {
        // Add new mapping
        midiMappings.push_back(mapping);
    }
}

void ParameterManager::removeMidiMapping(int controlNumber, int channel) {
    midiMappings.erase(
        std::remove_if(midiMappings.begin(), midiMappings.end(),
            [controlNumber, channel](const ParameterManager::MidiMapping& m) {
                return m.controlNumber == controlNumber &&
                       (channel == -1 || m.channel == channel);
            }),
        midiMappings.end());
}

ParameterManager::MidiMapping* ParameterManager::findMidiMapping(int controlNumber, int channel) {
    for (auto& mapping : midiMappings) {
        if (mapping.controlNumber == controlNumber &&
            (channel == -1 || mapping.channel == channel)) {
            return &mapping;
        }
    }
    return nullptr;
}

void ParameterManager::clearAllMidiMappings() {
    midiMappings.clear();
}

void ParameterManager::generateMappingTemplate(const std::string& filename) {
    ofxXmlSettings xml;
    
    xml.addTag("midiMappings");
    xml.pushTag("midiMappings");
    
    // Add example mappings for all parameters
    int index = 0;
    int exampleCC = 0;
    
    // Luma Key Level
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "luma_key_level");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // X Displacement
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "x_displace");
    xml.setValue("minValue", -1.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", true);
    xml.popTag(); // pop mapping
    index++;
    
    // Y Displacement
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "y_displace");
    xml.setValue("minValue", -1.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", true);
    xml.popTag(); // pop mapping
    index++;
    
    // Z Frequency
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "z_frequency");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 0.1f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // X Frequency
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "x_frequency");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 0.1f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // Y Frequency
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "y_frequency");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 0.1f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // Zoom
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "zoom");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // Scale
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "scale");
    xml.setValue("minValue", 50);
    xml.setValue("maxValue", 200);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // Center X
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "center_x");
    xml.setValue("minValue", -1.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", true);
    xml.popTag(); // pop mapping
    index++;
    
    // Center Y
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "center_y");
    xml.setValue("minValue", -1.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", true);
    xml.popTag(); // pop mapping
    index++;
    
    // Z LFO Argument
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "z_lfo_arg");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // Z LFO Amplitude
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "z_lfo_amp");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // X LFO Argument
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "x_lfo_arg");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // X LFO Amplitude
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "x_lfo_amp");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // Y LFO Argument
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "y_lfo_arg");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // Y LFO Amplitude
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "y_lfo_amp");
    xml.setValue("minValue", 0.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    index++;
    
    // Rotate X
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "rotate_x");
    xml.setValue("minValue", -180.0f);
    xml.setValue("maxValue", 180.0f);
    xml.setValue("bipolar", true);
    xml.popTag(); // pop mapping
    index++;
    
    // Rotate Y
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "rotate_y");
    xml.setValue("minValue", -180.0f);
    xml.setValue("maxValue", 180.0f);
    xml.setValue("bipolar", true);
    xml.popTag(); // pop mapping
    index++;
    
    // Rotate Z
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "rotate_z");
    xml.setValue("minValue", -180.0f);
    xml.setValue("maxValue", 180.0f);
    xml.setValue("bipolar", true);
    xml.popTag(); // pop mapping
    index++;
    
    // Global X Displacement
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "global_x_displace");
    xml.setValue("minValue", -1.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", true);
    xml.popTag(); // pop mapping
    index++;
    
    // Global Y Displacement
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "global_y_displace");
    xml.setValue("minValue", -1.0f);
    xml.setValue("maxValue", 1.0f);
    xml.setValue("bipolar", true);
    xml.popTag(); // pop mapping
    index++;
    
    // Stroke Weight
    xml.addTag("mapping");
    xml.pushTag("mapping", index);
    xml.setValue("controlNumber", exampleCC++);
    xml.setValue("channel", 0);
    xml.setValue("paramId", "stroke_weight");
    xml.setValue("minValue", 0.5f);
    xml.setValue("maxValue", 5.0f);
    xml.setValue("bipolar", false);
    xml.popTag(); // pop mapping
    
    xml.popTag(); // pop midiMappings
    
    xml.saveFile(filename);
    ofLogNotice("ParameterManager") << "Generated MIDI mapping template at " << filename;
}
