#pragma once

#include "ofMain.h"
#include "SimpleFFT.h"
#include <mutex>

//==============================================================================
// FFT Band definitions (8 bands)
//==============================================================================
enum class FFTBand {
    SUB_BASS = 0,    // 20-60 Hz
    BASS = 1,        // 60-120 Hz
    LOW_MID = 2,     // 120-250 Hz
    MID = 3,         // 250-500 Hz
    HIGH_MID = 4,    // 500-2000 Hz
    HIGH = 5,        // 2000-4000 Hz
    VERY_HIGH = 6,   // 4000-8000 Hz
    PRESENCE = 7,    // 8000-16000 Hz
    COUNT = 8
};

static const char* FFTBandNames[8] = {
    "Sub Bass (20-60Hz)",
    "Bass (60-120Hz)",
    "Low Mid (120-250Hz)",
    "Mid (250-500Hz)",
    "High Mid (500-2kHz)",
    "High (2k-4kHz)",
    "Very High (4k-8kHz)",
    "Presence (8k-16kHz)"
};

//==============================================================================
// Audio modulation for a single parameter
//==============================================================================
struct AudioModulation {
    bool enabled = false;
    int fftBand = 0;                    // 0-7 corresponding to FFTBand
    float amount = 0.0f;                // Modulation depth (-1 to 1, bipolar)
    bool useNormalization = true;       // Use normalized values
    float attack = 0.1f;                // Attack smoothing
    float release = 0.1f;               // Release smoothing
    float rangeScale = 1.0f;            // Scale factor for parameter range
    
    // Runtime state (not saved)
    float currentValue = 0.0f;
    
    float process(float fftValue, float deltaTime);
};

//==============================================================================
// Audio analyzer - FFT with 8 bands
// Self-contained implementation using SimpleFFT
//==============================================================================
class AudioAnalyzer : public ofBaseSoundInput {
public:
    AudioAnalyzer();
    ~AudioAnalyzer();
    
    // Setup and control
    void setup(int deviceIndex = 0, int sampleRate = 44100, int bufferSize = 512);
    void close();
    void update();  // Call every frame
    
    // Get FFT values (0-1 range, smoothed)
    float getBand(int bandIndex) const;
    float getBand(FFTBand band) const { return getBand(static_cast<int>(band)); }
    const std::array<float, 8>& getAllBands() const { return smoothedValues; }
    
    // Get raw FFT values (unsmoothed)
    float getRawBand(int bandIndex) const;
    
    // Settings
    void setEnabled(bool enabled);
    bool isEnabled() const { return bEnabled; }
    void setAmplitude(float amp) { amplitude = ofClamp(amp, 0.0f, 10.0f); }
    float getAmplitude() const { return amplitude; }
    void setSmoothing(float smooth) { smoothing = ofClamp(smooth, 0.0f, 0.99f); }
    float getSmoothing() const { return smoothing; }
    void setNormalization(bool norm) { normalization = norm; }
    bool getNormalization() const { return normalization; }
    void setPeakDecay(float decay) { peakDecay = ofClamp(decay, 0.8f, 0.999f); }
    float getPeakDecay() const { return peakDecay; }
    
    // Device management
    std::vector<std::string> getDeviceList() const;
    void setDevice(int deviceIndex);
    int getCurrentDevice() const { return currentDeviceIndex; }
    int getCurrentDeviceId() const { return currentDeviceId; }
    
    // Audio input callbacks
    void audioIn(ofSoundBuffer& buffer) override;
    void audioIn(float* input, int bufferSize, int nChannels) override;
    
    // Reset normalization ranges
    void resetNormalization();
    
    // Visualization helpers
    float getVolume() const { return currentVolume; }
    float getSmoothedVolume() const { return smoothedVolume; }
    bool isSilent() const { return currentVolume < 0.001f; }
    float getPeak(int bandIndex) const;
    
private:
    // Build the list of input device IDs
    void rebuildDeviceIdMap();
    void computeFFT();
    void computeBandValues();
    void updateNormalization();
    
    bool bEnabled = false;
    bool streamSetup = false;
    
    // Settings
    int currentDeviceIndex = 0;
    int currentDeviceId = -1;
    int sampleRate = 44100;
    int bufferSize = 512;
    int numBins = 256;  // Power of 2
    int fftSize = 1024; // Circular buffer size
    float amplitude = 1.0f;
    float smoothing = 0.5f;
    float peakDecay = 0.95f;
    bool normalization = true;
    static constexpr float NORMALIZATION_DECAY = 0.999f;
    
    // FFT bins (magnitude spectrum)
    std::vector<float> fftBins;
    
    // 8 bands for display
    std::array<float, 8> bandValues;      // Current frame
    std::array<float, 8> smoothedValues;  // Smoothed
    std::array<float, 8> peakValues;      // Peak hold
    std::array<float, 8> minValues;       // For normalization
    std::array<float, 8> maxValues;       // For normalization
    
    // Volume tracking
    float currentVolume = 0.0f;
    float smoothedVolume = 0.0f;
    
    // Sound stream
    ofSoundStream soundStream;
    
    // Device ID mapping
    std::vector<int> inputDeviceIds;
    
    // Audio input buffer (circular buffer for continuous FFT)
    std::vector<float> audioBuffer;
    size_t bufferWriteIndex = 0;
    std::mutex audioMutex;
    
    // FFT input/output buffers
    std::vector<float> fftInputBuffer;
    std::vector<float> fftWindow;
};
