#include "MidiManager.h"
#include "ParameterManager.h"

MidiManager::MidiManager(ParameterManager* paramManager) : paramManager(paramManager) {
    // Initialize MIDI control active states
    midiActiveControls.resize(17, false);
}

MidiManager::~MidiManager() {
    // Clean up
    disconnectCurrentDevice();
    midiIn.removeListener(this);
}

void MidiManager::setup() {
    // Register as a MIDI listener
    midiIn.addListener(this);
    
    // Initial device scan
    scanForDevices();
    
    // Try to connect to preferred device if set
    if (!preferredDeviceName.empty()) {
        connectToDevice(preferredDeviceName);
    }
    // Otherwise try the first available device
    else if (!availableDevices.empty()) {
        connectToDevice(0);
    }
}

void MidiManager::update() {
    // Check for device changes periodically (hot-plugging)
    float currentTime = ofGetElapsedTimef();
    if (currentTime - lastDeviceScanTime > DEVICE_SCAN_INTERVAL) {
        scanForDevices();
        
        // If we're not connected but have a preferred device available, connect to it
        if (!isDeviceConnected && !preferredDeviceName.empty()) {
            for (size_t i = 0; i < availableDevices.size(); i++) {
                if (availableDevices[i] == preferredDeviceName) {
                    connectToDevice(static_cast<int>(i));
                    break;
                }
            }
        }
        
        lastDeviceScanTime = currentTime;
    }
}

void MidiManager::scanForDevices() {
    // Store available MIDI devices
    availableDevices = midiIn.getInPortList();
    
    // Check if our current device is still available
    if (isDeviceConnected && currentDeviceIndex >= 0) {
        bool deviceStillExists = false;
        std::string currentName = midiIn.getInPortName(currentDeviceIndex);
        
        for (const auto& name : availableDevices) {
            if (name == currentName) {
                deviceStillExists = true;
                break;
            }
        }
        
        if (!deviceStillExists) {
            disconnectCurrentDevice();
        }
    }
}

bool MidiManager::connectToDevice(int deviceIndex) {
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(availableDevices.size())) {
        return false;
    }
    
    // Disconnect current device if any
    disconnectCurrentDevice();
    
    // Try to open the new device
    try {
        midiIn.openPort(deviceIndex);
        currentDeviceIndex = deviceIndex;
        isDeviceConnected = true;
        preferredDeviceName = availableDevices[deviceIndex];
        
        // Don't ignore sysex, timing, and active sense
        midiIn.ignoreTypes(false, false, false);
        
        ofLogNotice("MidiManager") << "Connected to MIDI device: " << preferredDeviceName;
        return true;
    }
    catch (const std::exception& e) {
        ofLogError("MidiManager") << "Failed to connect to MIDI device: " << e.what();
        return false;
    }
}

bool MidiManager::connectToDevice(const std::string& deviceName) {
    for (size_t i = 0; i < availableDevices.size(); i++) {
        if (availableDevices[i] == deviceName) {
            return connectToDevice(static_cast<int>(i));
        }
    }
    return false;
}

void MidiManager::disconnectCurrentDevice() {
    if (isDeviceConnected) {
        midiIn.closePort();
        isDeviceConnected = false;
        currentDeviceIndex = -1;  // Reset the index when disconnecting
        ofLogNotice("MidiManager") << "Disconnected from MIDI device";
    }
}

void MidiManager::newMidiMessage(ofxMidiMessage& message) {
    // Add the message to the queue
    midiMessages.push_back(message);
    
    // Remove old messages if queue is too large
    while (midiMessages.size() > maxMessages) {
        midiMessages.erase(midiMessages.begin());
    }
    
    // Process the message
    if (message.status == MIDI_CONTROL_CHANGE) {
        processControlChange(message);
    }
}

std::vector<ofxMidiMessage> MidiManager::getRecentMessages() const {
    return midiMessages;
}

void MidiManager::processControlChange(const ofxMidiMessage& message) {
    // Record button
    if (message.control == 60) {
        if (message.value == 127) {
            paramManager->startRecording();
        } else if (message.value == 0) {
            paramManager->stopRecording();
        }
        return;
    }
    
    // Reset button
    if (message.control == 58 && message.value == 127) {
        paramManager->resetToDefaults();
        for (size_t i = 0; i < midiActiveControls.size(); ++i) {
            midiActiveControls[i] = false;
        }
        return;
    }
    
    // First check for mapped controls (this takes precedence over hardcoded mappings)
    ParameterManager::MidiMapping* mapping = paramManager->findMidiMapping(message.control, message.channel);
    if (mapping) {
        float normalizedValue = normalizeValue(message.value, mapping->bipolar);
        
        // Scale to parameter range
        float paramValue = mapping->minValue + normalizedValue * (mapping->maxValue - mapping->minValue);
        
        // Apply to parameter using a parameter ID lookup
        applyParameterValue(mapping->paramId, paramValue);
        return;
    }
    
    // Fallback to hardcoded parameter control mappings
    float normalizedValue;
    
    switch (message.control) {
        // Luma key level
        case 16: {
            normalizedValue = normalizeValue(message.value);
            if (abs(normalizedValue - paramManager->getLumaKeyLevel()) < CONTROL_THRESHOLD) {
                midiActiveControls[0] = true;
            }
            if (midiActiveControls[0]) {
                paramManager->setLumaKeyLevel(normalizedValue);
            }
            break;
        }
        
        // X Displacement
        case 17: {
            normalizedValue = normalizeValue(message.value, true);
            if (abs(normalizedValue - paramManager->getXDisplace()) < CONTROL_THRESHOLD) {
                midiActiveControls[1] = true;
            }
            if (midiActiveControls[1]) {
                paramManager->setXDisplace(normalizedValue);
            }
            break;
        }
        
        // Y Displacement
        case 18: {
            normalizedValue = normalizeValue(message.value, true);
            if (abs(normalizedValue - paramManager->getYDisplace()) < CONTROL_THRESHOLD) {
                midiActiveControls[2] = true;
            }
            if (midiActiveControls[2]) {
                paramManager->setYDisplace(normalizedValue);
            }
            break;
        }
        
        // Z Frequency
        case 19: {
            normalizedValue = normalizeValue(message.value);
            if (abs(normalizedValue - paramManager->getZFrequency()) < CONTROL_THRESHOLD) {
                midiActiveControls[3] = true;
            }
            if (midiActiveControls[3]) {
                paramManager->setZFrequency(normalizedValue);
            }
            break;
        }
        
        // X Frequency
        case 20: {
            normalizedValue = normalizeValue(message.value);
            if (abs(normalizedValue - paramManager->getXFrequency()) < CONTROL_THRESHOLD) {
                midiActiveControls[4] = true;
            }
            if (midiActiveControls[4]) {
                paramManager->setXFrequency(normalizedValue);
            }
            break;
        }
        
        // Y Frequency
        case 21: {
            normalizedValue = normalizeValue(message.value);
            if (abs(normalizedValue - paramManager->getYFrequency()) < CONTROL_THRESHOLD) {
                midiActiveControls[5] = true;
            }
            if (midiActiveControls[5]) {
                paramManager->setYFrequency(normalizedValue);
            }
            break;
        }
        
        // Zoom
        case 22: {
            normalizedValue = normalizeValue(message.value);
            if (abs(normalizedValue - paramManager->getZoom()) < CONTROL_THRESHOLD) {
                midiActiveControls[6] = true;
            }
            if (midiActiveControls[6]) {
                paramManager->setZoom(normalizedValue);
            }
            break;
        }
        
        // Toggle controls
        case 49: // Triangle mesh
            if (message.value == 127) {
                paramManager->setMeshType(MeshType::TriangleGrid);
                paramManager->setWireframeEnabled(false);
            }
            break;
            
        case 50: // Horizontal lines
            if (message.value == 127) {
                paramManager->setMeshType(MeshType::HorizontalLines);
            }
            break;
            
        case 51: // Vertical lines
            if (message.value == 127) {
                paramManager->setMeshType(MeshType::VerticalLines);
            }
            break;
            
        case 52: // Triangle wireframe
            if (message.value == 127) {
                paramManager->setMeshType(MeshType::TriangleWireframe);
                paramManager->setWireframeEnabled(true);
            }
            break;
            
        // Effect toggles
        case 53: // Bright mode
            if (message.value == 127) {
                paramManager->setBrightEnabled(!paramManager->isBrightEnabled());
            }
            break;
            
        case 54: // Invert
            if (message.value == 127) {
                paramManager->setInvertEnabled(!paramManager->isInvertEnabled());
            }
            break;
            
        case 55: // Stroke weight
            if (message.value == 127) {
                paramManager->setStrokeWeightEnabled(!paramManager->isStrokeWeightEnabled());
            }
            break;
            
        case 56: // Black & White
            if (message.value == 127) {
                paramManager->setBlackAndWhiteEnabled(!paramManager->isBlackAndWhiteEnabled());
            }
            break;
            
        case 57: // Luma Key
            if (message.value == 127) {
                paramManager->setLumaKeyEnabled(!paramManager->isLumaKeyEnabled());
            }
            break;
    }
}

void MidiManager::applyParameterValue(const std::string& paramId, float value) {
    // Using a map of parameter IDs to setter functions would be cleaner,
    // but this approach works for now
    if (paramId == "luma_key_level") {
        paramManager->setLumaKeyLevel(value);
    }
    else if (paramId == "x_displace") {
        paramManager->setXDisplace(value);
    }
    else if (paramId == "y_displace") {
        paramManager->setYDisplace(value);
    }
    else if (paramId == "z_frequency") {
        paramManager->setZFrequency(value);
    }
    else if (paramId == "x_frequency") {
        paramManager->setXFrequency(value);
    }
    else if (paramId == "y_frequency") {
        paramManager->setYFrequency(value);
    }
    else if (paramId == "zoom") {
        paramManager->setZoom(value);
    }
    else if (paramId == "center_x") {
        paramManager->setCenterX(value);
    }
    else if (paramId == "center_y") {
        paramManager->setCenterY(value);
    }
    else if (paramId == "z_lfo_arg") {
        paramManager->setZLfoArg(value);
    }
    else if (paramId == "z_lfo_amp") {
        paramManager->setZLfoAmp(value);
    }
    else if (paramId == "x_lfo_arg") {
        paramManager->setXLfoArg(value);
    }
    else if (paramId == "x_lfo_amp") {
        paramManager->setXLfoAmp(value);
    }
    else if (paramId == "y_lfo_arg") {
        paramManager->setYLfoArg(value);
    }
    else if (paramId == "y_lfo_amp") {
        paramManager->setYLfoAmp(value);
    }
    else if (paramId == "rotate_x") {
        paramManager->setRotateX(value);
    }
    else if (paramId == "rotate_y") {
        paramManager->setRotateY(value);
    }
    else if (paramId == "rotate_z") {
        paramManager->setRotateZ(value);
    }
    else if (paramId == "global_x_displace") {
        paramManager->setGlobalXDisplace(value);
    }
    else if (paramId == "global_y_displace") {
        paramManager->setGlobalYDisplace(value);
    }
    else if (paramId == "stroke_weight") {
        paramManager->setStrokeWeight(value);
    }
}

float MidiManager::normalizeValue(int value, bool centered) const {
    if (centered) {
        // For values that should be centered around 0 (-1 to 1)
        return (value - MIDI_MAGIC) / MIDI_MAGIC;
    } else {
        // For values that range from 0 to 1
        return value / 127.0f;
    }
}

std::vector<std::string> MidiManager::getAvailableDevices() const {
    return availableDevices;
}

std::string MidiManager::getCurrentDeviceName() const {
    if (isDeviceConnected && currentDeviceIndex >= 0) {
        // Use const_cast to work around non-const method
        return const_cast<ofxMidiIn&>(midiIn).getInPortName(currentDeviceIndex);
    }
    return "";
}

int MidiManager::getCurrentDeviceIndex() const {
    return currentDeviceIndex;
}

void MidiManager::loadSettings(ofxXmlSettings& xml) {
    // Get preferred device name from XML
    preferredDeviceName = xml.getValue("midi:preferredDevice", "");
    
    if (!preferredDeviceName.empty()) {
        ofLogNotice("MidiManager") << "Loaded MIDI settings, preferred device: " << preferredDeviceName;
    }
}

void MidiManager::saveSettings(ofxXmlSettings& xml) const {
    // Save preferred device name to XML
    xml.setValue("midi:preferredDevice", preferredDeviceName);
    ofLogNotice("MidiManager") << "Saved MIDI settings";
}
