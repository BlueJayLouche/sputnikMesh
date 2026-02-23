#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxXmlSettings.h" // Changed from ofXml.h to ofxXmlSettings.h

// Forward declaration
class ParameterManager;
struct MidiMapping;

class MidiManager : public ofxMidiListener {
public:
    MidiManager(ParameterManager* paramManager);
    ~MidiManager();
    
    // Core methods
    void setup();
    void update();
    
    // MIDI device handling
    void scanForDevices();
    bool connectToDevice(int deviceIndex);
    bool connectToDevice(const std::string& deviceName);
    void disconnectCurrentDevice();
    
    // MIDI message handling
    void newMidiMessage(ofxMidiMessage& message) override;
    
    std::vector<ofxMidiMessage> getRecentMessages() const;
    
    // Device info
    std::vector<std::string> getAvailableDevices() const;
    std::string getCurrentDeviceName() const;
    int getCurrentDeviceIndex() const;
    
    // Settings
    void loadSettings(ofxXmlSettings& xml); // Removed const since ofxXmlSettings methods modify state
    void saveSettings(ofxXmlSettings& xml) const; // Changed from ofXml to ofxXmlSettings
    
private:
    // MIDI input
    mutable ofxMidiIn midiIn;
    std::vector<ofxMidiMessage> midiMessages;
    size_t maxMessages = 10;
    
    // Device management
    std::vector<std::string> availableDevices;
    int currentDeviceIndex = -1;
    std::string preferredDeviceName;
    
    // For hot-plugging
    bool isDeviceConnected = false;
    float lastDeviceScanTime = 0;
    const float DEVICE_SCAN_INTERVAL = 2.0f; // seconds
    
    // Control values
    static constexpr float MIDI_MAGIC = 63.50f;
    static constexpr float CONTROL_THRESHOLD = 0.04f;
    std::vector<bool> midiActiveControls;
    
    // Reference to parameter manager for updating values
    ParameterManager* paramManager;
    
    // Helper methods
    void processControlChange(const ofxMidiMessage& message);
    void applyParameterValue(const std::string& paramId, float value);
    float normalizeValue(int value, bool centered = false) const;
};
