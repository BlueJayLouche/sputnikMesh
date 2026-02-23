#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetFrameRate(30); // Default framerate, will adjust based on performance mode later
    ofSetVerticalSync(true);
    ofBackground(0);
    ofHideCursor();
    
    // Disable ArbTex for normalized texture coordinates
    ofDisableArbTex();
    
    // Initialize parameter manager first
    paramManager = std::make_unique<ParameterManager>();
    paramManager->setup();
    
    // Now it's safe to check performance mode after paramManager is initialized
    bool performanceMode = paramManager->isPerformanceModeEnabled();
    
    // Set framerate based on performance mode
    ofSetFrameRate(performanceMode ? 24 : 30);
    
    // Initialize mesh generator
    meshGenerator = std::make_unique<MeshGenerator>(paramManager.get());
    meshGenerator->setup(width, height);
    
    // Initialize audio reactivity manager BEFORE loading settings
    audioManager = std::make_unique<AudioReactivityManager>();
    audioManager->setup(paramManager.get(), performanceMode);
    
    // Initialize MIDI manager
    midiManager = std::make_unique<MidiManager>(paramManager.get());
    
    // Load settings with proper error handling
    ofxXmlSettings xml;
    if (xml.load(ofToDataPath("settings.xml"))) {
        try {
            // First load parameter settings
            paramManager->loadFromXml(xml);
            
            // Check performance mode again after loading settings
            performanceMode = paramManager->isPerformanceModeEnabled();
            
            // Then load MIDI settings
            midiManager->loadSettings(xml);
            
            // Then safely load audio settings
            if (xml.tagExists("audioReactivity")) {
                audioManager->loadFromXml(xml);
            } else {
                ofLogNotice("ofApp") << "No audio settings found, using defaults";
            }
        } catch (const std::exception& e) {
            ofLogError("ofApp") << "Error loading settings: " << e.what();
        } catch (...) {
            ofLogError("ofApp") << "Unknown error loading settings, using defaults";
        }
    } else {
        ofLogNotice("ofApp") << "Settings file not found, using defaults";
    }
    
    midiManager->setup();
    
    // Setup camera
    setupCamera(width, height);

    // Allocate FBOs
    allocateFbos();
    
    // Load appropriate shaders based on renderer
    if(ofIsGLProgrammableRenderer()){
        shaderDisplace.load("shadersGL3/displacement");
        ofLogNotice("ofApp") << "Loaded GL3 shader";
    } else {
        #ifdef TARGET_OPENGLES
            shaderDisplace.load("shadersES2/displacement");
            ofLogNotice("ofApp") << "Loaded ES2 shader";
        #else
            shaderDisplace.load("shadersGL2/displacement");
            ofLogNotice("ofApp") << "Loaded GL2 shader";
        #endif
    }
    
    // Setup noise images
//    xNoiseImage.allocate(180, 120, OF_IMAGE_GRAYSCALE);
//    yNoiseImage.allocate(180, 120, OF_IMAGE_GRAYSCALE);
//    zNoiseImage.allocate(180, 120, OF_IMAGE_GRAYSCALE);
    
    // Update noise texture allocation based on performance mode (after settings are loaded)
    if (performanceMode) {
        // Use smaller noise textures for better performance
        xNoiseImage.allocate(45, 30, OF_IMAGE_GRAYSCALE); // Half size in performance mode
        yNoiseImage.allocate(45, 30, OF_IMAGE_GRAYSCALE);
        zNoiseImage.allocate(45, 30, OF_IMAGE_GRAYSCALE);
    } else {
        // Standard size
        xNoiseImage.allocate(90, 60, OF_IMAGE_GRAYSCALE);
        yNoiseImage.allocate(90, 60, OF_IMAGE_GRAYSCALE);
        zNoiseImage.allocate(90, 60, OF_IMAGE_GRAYSCALE);
    }
    
    // Ensure texture settings are correct for shaders
    xNoiseImage.getTexture().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    yNoiseImage.getTexture().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    zNoiseImage.getTexture().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    
    // Setup audio reactivity
    setupAudioReactivity();
    
    audioManager->setEnabled(true);
}

//--------------------------------------------------------------
void ofApp::update() {
    // Update subsystems
    midiManager->update();
    paramManager->update();
    
    // Update camera feed
    updateCamera();
    
    // Update noise images
//    xNoiseImage = generatePerlinNoise(xLfoArg, paramManager->getXFrequency(), xNoiseImage);
//    yNoiseImage = generatePerlinNoise(yLfoArg, paramManager->getYFrequency(), yNoiseImage);
//    zNoiseImage = generatePerlinNoise(zLfoArg, paramManager->getZFrequency(), zNoiseImage);
    
    // Update noise images with performance considerations
    bool performanceMode = paramManager->isPerformanceModeEnabled();
    int updateInterval = performanceMode ? paramManager->getNoiseUpdateInterval() : 2;

    // Only update noise textures every N frames based on performance settings
    if (ofGetFrameNum() % updateInterval == 0) {
        xNoiseImage = generatePerlinNoise(xLfoArg, paramManager->getXFrequency(), xNoiseImage);
        
        // In performance mode, stagger updates to avoid CPU spikes
        if (!performanceMode || ofGetFrameNum() % (updateInterval * 2) == 0) {
            yNoiseImage = generatePerlinNoise(yLfoArg, paramManager->getYFrequency(), yNoiseImage);
        }
        
        if (!performanceMode || ofGetFrameNum() % (updateInterval * 3) == 0) {
            zNoiseImage = generatePerlinNoise(zLfoArg, paramManager->getZFrequency(), zNoiseImage);
        }
    }
    
    // Update LFO arguments
    xLfoArg += paramManager->getXLfoArg();
    yLfoArg += paramManager->getYLfoArg();
    zLfoArg += paramManager->getZLfoArg();
    
    // Update mesh if needed
    int currentScale = paramManager->getScale();
    if (currentScale != meshGenerator->getMesh().getNumVertices() / 6) { // Triangle mode has 6 vertices per grid cell
        meshGenerator->updateMeshFromParameters();
    }
    
//    audioManager->update();
    
    // Conditional audio processing based on performance mode
    if (!performanceMode || ofGetFrameNum() % 2 == 0) {
        audioManager->update();
    }
    
    float currentTime = ofGetElapsedTimef();
    static float lastTime = currentTime;
    float frameDuration = currentTime - lastTime;
    lastTime = currentTime;

    performanceMetrics[metricIndex] = frameDuration;
    metricIndex = (metricIndex + 1) % 60;
}

//--------------------------------------------------------------
void ofApp::draw() {
    // Calculate display values with parameter lock influence
    float lumaKeyLevel = paramManager->getLumaKeyLevel() + keyControls.lumaKey;
    float xDisplace = 100.0f * (paramManager->getXDisplace() + keyControls.xDisplace);
    float yDisplace = 100.0f * (paramManager->getYDisplace() + keyControls.yDisplace);
    float zFrequency = 0.03f * paramManager->getZFrequency() + keyControls.zFrequency;
    float xFrequency = 0.015f * paramManager->getXFrequency() + keyControls.xFrequency;
    float yFrequency = 0.02f * paramManager->getYFrequency() + keyControls.yFrequency;
    float zoom = paramManager->getZoom() * 480 + keyControls.zoom;
    
    float zLfoArg = paramManager->getZLfoArg() + keyControls.zLfoArg;
    float xLfoArg = paramManager->getXLfoArg() + keyControls.xLfoArg;
    float yLfoArg = paramManager->getYLfoArg() + keyControls.yLfoArg;
    
    float centerX = -960.0f * paramManager->getCenterX() + keyControls.centerX;
    float centerY = -960.0f * paramManager->getCenterY() + keyControls.centerY;
    
    float zLfoAmp = 0.25f * paramManager->getZLfoAmp() + keyControls.zLfoAmp;
    float xLfoAmp = ofGetWidth() * 0.25f * paramManager->getXLfoAmp() + keyControls.xLfoAmp;
    float yLfoAmp = ofGetHeight() * 0.25f * paramManager->getYLfoAmp() + keyControls.yLfoAmp;
    
    // Update camera FBO if needed
    if (camera.isInitialized() && camera.isFrameNew()) {
        cameraFbo.begin();
        ofClear(0, 0, 0, 255);
        camera.draw(0, 0, width, height);
        cameraFbo.end();
    }
    
    // Now render the effect in the main FBO
    mainFbo.begin();
    
    // Set background
    if (paramManager->isInvertEnabled()) {
        ofBackground(255);
    } else {
        ofBackground(0);
    }
    
    shaderDisplace.begin();
    
    // Bind textures with explicit texture units
    shaderDisplace.setUniformTexture("tex0", cameraFbo.getTexture(), 0);
    shaderDisplace.setUniformTexture("x_noise_image", xNoiseImage.getTexture(), 1);
    shaderDisplace.setUniformTexture("y_noise_image", yNoiseImage.getTexture(), 2);
    shaderDisplace.setUniformTexture("z_noise_image", zNoiseImage.getTexture(), 3);
    
    // Set luma key and other basic parameters
    shaderDisplace.setUniform1f("luma_key_level", lumaKeyLevel);
    shaderDisplace.setUniform1f("invert_switch", paramManager->isInvertEnabled() ? 1.0f : 0.0f);
    shaderDisplace.setUniform1f("b_w_switch", paramManager->isBlackAndWhiteEnabled() ? 1.0f : 0.0f);
    shaderDisplace.setUniform1i("luma_switch", paramManager->isLumaKeyEnabled() ? 1 : 0);
    shaderDisplace.setUniform1i("bright_switch", paramManager->isBrightEnabled() ? 1 : 0);
    
    // Set displacement values
    shaderDisplace.setUniform2f("xy", ofVec2f(xDisplace, yDisplace));
    
    // Set dimensions
    shaderDisplace.setUniform1i("width", width);
    shaderDisplace.setUniform1i("height", height);
    
    // Set LFO parameters for X, Y, Z
    shaderDisplace.setUniform1f("z_lfo_amp", zLfoAmp);
    shaderDisplace.setUniform1f("z_lfo_arg", zLfoArg);
    shaderDisplace.setUniform1f("z_lfo_other", zFrequency);
    
    shaderDisplace.setUniform1f("x_lfo_amp", xLfoAmp);
    shaderDisplace.setUniform1f("x_lfo_arg", xLfoArg);
    shaderDisplace.setUniform1f("x_lfo_other", xFrequency);
    
    shaderDisplace.setUniform1f("y_lfo_amp", yLfoAmp);
    shaderDisplace.setUniform1f("y_lfo_arg", yLfoArg);
    shaderDisplace.setUniform1f("y_lfo_other", yFrequency);
    
    // Set center offset
    shaderDisplace.setUniform2f("xy_offset", ofVec2f(centerX, centerY));
    
    // Set LFO shapes
    shaderDisplace.setUniform1i("yLfoShape", static_cast<int>(paramManager->getYLfoShape()));
    shaderDisplace.setUniform1i("xLfoShape", static_cast<int>(paramManager->getXLfoShape()));
    shaderDisplace.setUniform1i("zLfoShape", static_cast<int>(paramManager->getZLfoShape()));
    
    // Set modulation toggles
    shaderDisplace.setUniform1i("y_phasemod_switch", paramManager->isYPhaseModEnabled());
    shaderDisplace.setUniform1i("x_phasemod_switch", paramManager->isXPhaseModEnabled());
    shaderDisplace.setUniform1i("z_phasemod_switch", paramManager->isZPhaseModEnabled());
    
    shaderDisplace.setUniform1i("y_ringmod_switch", paramManager->isYRingModEnabled());
    shaderDisplace.setUniform1i("x_ringmod_switch", paramManager->isXRingModEnabled());
    shaderDisplace.setUniform1i("z_ringmod_switch", paramManager->isZRingModEnabled());
    
    // Set line width for stroke
    glLineWidth(paramManager->getStrokeWeight());
    
    // Draw the mesh
    ofPushMatrix();
    ofTranslate(0, 0, zoom);
    
    if (paramManager->isWireframeEnabled()) {
        meshGenerator->getMesh().drawWireframe();
    } else {
        meshGenerator->getMesh().draw();
    }
    
    ofPopMatrix();
    
    shaderDisplace.end();
    mainFbo.end();
    
    // Draw the final output with transformations
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2, 100);
    ofRotateXRad(paramManager->getRotateX());
    ofRotateYRad(paramManager->getRotateY());
    ofRotateZRad(paramManager->getRotateZ());
    
    mainFbo.draw(
        -ofGetWidth()/2 + paramManager->getGlobalXDisplace(),
        -ofGetHeight()/2 + paramManager->getGlobalYDisplace(),
        720, 480
    );
    
    ofPopMatrix();
    
    // Draw debug info if enabled
    if (paramManager->isDebugEnabled()) {
        drawDebugInfo();
    }
    
    // Draw audio visualization if debug is enabled
    drawAudioVisualization();
    
    // Draw audio device selection UI
    drawAudioDevicesUI();
    
    // Draw video device selection UI
    drawVideoDevicesUI();
    
}

//--------------------------------------------------------------
//void ofApp::exit() {
//    ofLogNotice("ofApp") << "Shutting down audio system...";
//    
//    // Properly release audio resources
//    if (audioManager) {
//        audioManager->exit();
//    }
//    
//    // Call parent exit to ensure other resources are released
//    ofBaseApp::exit();
//}

//--------------------------------------------------------------
void ofApp::exit() {
    // Load existing settings first (if any)
    ofxXmlSettings xml;
    xml.loadFile(ofToDataPath("settings.xml")); // Even if it fails, we'll create a new one
    
    // Save parameters
    paramManager->saveToXml(xml);
    
    // Add MIDI settings
    midiManager->saveSettings(xml);
    
    // Add audio reactivity settings
    audioManager->saveToXml(xml);
    
    // Save the combined file
    if (xml.saveFile(ofToDataPath("settings.xml"))) {
        ofLogNotice("ofApp") << "Settings saved successfully";
    } else {
        ofLogError("ofApp") << "Failed to save settings";
    }
    
    // Clean up resources
    midiManager.reset();
}

//--------------------------------------------------------------
void ofApp::setupCamera(int w, int h) {
    // List available devices if we haven't already
    if (videoDevices.empty()) {
        listVideoDevices();
    }
    
    // Get settings from parameter manager
    int deviceID = paramManager->getVideoDeviceID();
    std::string devicePath = paramManager->getVideoDevicePath();
    std::string format = paramManager->getVideoFormat();
    bool performanceMode = paramManager->isPerformanceModeEnabled();
    
    // Get desired dimensions, possibly adjusted for performance
    int desiredWidth = paramManager->getVideoWidth();
    int desiredHeight = paramManager->getVideoHeight();
    
    // In performance mode, limit resolution if it's too high
    if (performanceMode && desiredWidth > 640) {
        float aspect = (float)desiredHeight / desiredWidth;
        desiredWidth = 640;
        desiredHeight = round(desiredWidth * aspect);
        ofLogNotice("ofApp") << "Performance Mode: Camera resolution limited to "
                           << desiredWidth << "x" << desiredHeight;
    }
    
    int frameRate = performanceMode ? 24 : paramManager->getVideoFrameRate();
    
    // Set device ID if we have a valid one
    if (deviceID >= 0 && deviceID < videoDevices.size()) {
        camera.setDeviceID(deviceID);
        ofLogNotice("ofApp") << "Setting camera device ID to: " << deviceID;
    } else {
        camera.setDeviceID(0); // Use first available device
        ofLogNotice("ofApp") << "Using default camera device ID: 0";
    }
    
    // Set desired frame rate
    camera.setDesiredFrameRate(frameRate);
    ofLogNotice("ofApp") << "Setting camera frame rate to: " << frameRate;
    
    // On Linux/Raspberry Pi, try to set the format using V4L2 before opening
    #ifdef TARGET_LINUX
    if (!format.empty() && !devicePath.empty()) {
        uint32_t formatCode = V4L2Helper::formatNameToCode(format);
        // Try setting the format
        ofLogNotice("ofApp") << "Trying to set V4L2 format: " << format << " ("
                           << formatCode << ") " << desiredWidth << "x" << desiredHeight;
        V4L2Helper::setFormat(devicePath, formatCode, desiredWidth, desiredHeight);
    }
    #endif
    
    // Use desired dimensions if specified, otherwise use passed values
    int useWidth = (desiredWidth > 0) ? desiredWidth : w;
    int useHeight = (desiredHeight > 0) ? desiredHeight : h;
    
    ofLogNotice("ofApp") << "Initializing camera with size: " << useWidth << "x" << useHeight;
    
    // Initialize the grabber
    camera.initGrabber(useWidth, useHeight);
    
    // Update parameter manager with actual dimensions (may differ from requested)
    paramManager->setVideoWidth(camera.getWidth());
    paramManager->setVideoHeight(camera.getHeight());
    
    ofLogNotice("ofApp") << "Camera initialized: "
                       << camera.getWidth() << "x" << camera.getHeight()
                       << " @ " << frameRate << "fps";
    
    // Get and log the actual format that was set
    #ifdef TARGET_LINUX
    if (!devicePath.empty()) {
        V4L2Helper::VideoFormat currentFormat = V4L2Helper::getCurrentFormat(devicePath);
        ofLogNotice("ofApp") << "Current format: " << currentFormat.name
                           << " (" << currentFormat.fourcc << ")";
    }
    #endif
    
    // Check if camera was initialized successfully
    if (!camera.isInitialized()) {
        ofLogError("ofApp") << "Failed to initialize camera!";
    }
}

//--------------------------------------------------------------
void ofApp::updateCamera() {
    camera.update();
    
    if (camera.isFrameNew()) {
        // Render camera to RGBA FBO
        cameraFbo.begin();
        ofClear(0, 0, 0, 255);
        camera.draw(0, 0, width, height);
        cameraFbo.end();
        
        // Corner crop and stretch to preserve HD aspect ratio if needed
        if (hdmiAspectRatioEnabled) {
            aspectFixFbo.begin();
            cameraFbo.draw(0, 0, 853, 480);
            aspectFixFbo.end();
        }
    }
}

//--------------------------------------------------------------
void ofApp::allocateFbos() {
    // Check for performance mode
    bool performanceMode = paramManager->isPerformanceModeEnabled();
    
    // Potentially reduce FBO size for performance
    int fboWidth = width;
    int fboHeight = height;
    
    if (performanceMode) {
        // Scale down FBOs in performance mode if they're high resolution
        if (width > 640) {
            float aspect = (float)height / width;
            fboWidth = 640;
            fboHeight = round(fboWidth * aspect);
            ofLogNotice("ofApp") << "Performance Mode: FBOs scaled down to "
                               << fboWidth << "x" << fboHeight;
        }
    }
    
    // Allocate main FBO with explicit RGBA format
    ofFboSettings settings;
    settings.width = fboWidth;
    settings.height = fboHeight;
    settings.internalformat = GL_RGBA;  // Explicit RGBA format
    settings.useDepth = false;          // Don't need depth for 2D effects
    settings.numSamples = 0;            // No multisampling needed
    
    // Setup main FBO
    mainFbo.allocate(settings);
    mainFbo.begin();
    ofClear(0, 0, 0, 255);
    mainFbo.end();
    
    // Setup aspect ratio correction FBO with same format
    aspectFixFbo.allocate(settings);
    aspectFixFbo.begin();
    ofClear(0, 0, 0, 255);
    aspectFixFbo.end();
    
    // Setup a new FBO specifically for camera input conversion
    cameraFbo.allocate(settings);
    cameraFbo.begin();
    ofClear(0, 0, 0, 255);
    cameraFbo.end();
}

//--------------------------------------------------------------
ofImage ofApp::generatePerlinNoise(float theta, float resolution, ofImage& noiseImage) {
    // Get performance mode setting
    bool performanceMode = paramManager->isPerformanceModeEnabled();
    
    // In performance mode, use higher skip values to process fewer pixels
    int skip = performanceMode ? 4 : 2; // Process every 4th pixel in performance mode
    
    // Scale parameters
    resolution *= 0.05f;
    theta *= 0.1f;
    
    // Get access to image pixels
    ofPixels& pixels = noiseImage.getPixels();
    int w = noiseImage.getWidth();
    int h = noiseImage.getHeight();
    
    // Generate noise
    for (int y = 0; y < h; y += skip) {
        for (int x = 0; x < w; x += skip) {
            int i = y * w + x;
            float noiseValue = ofNoise(x * resolution, y * resolution, theta);
            
            // Fill a block of pixels with the same value for performance
            for (int dy = 0; dy < skip && y + dy < h; dy++) {
                for (int dx = 0; dx < skip && x + dx < w; dx++) {
                    int idx = (y + dy) * w + (x + dx);
                    if (idx < pixels.size()) {
                        pixels[idx] = 255 * noiseValue;
                    }
                }
            }
        }
    }
    
    // Update the image with the new pixels
    noiseImage.update();
    return noiseImage;
}

//--------------------------------------------------------------
void ofApp::drawAudioDevicesUI() {
    if (!paramManager->isDebugEnabled() || !audioManager->isEnabled()) return;
    
    ofPushStyle();
    
    // Position and size for the device selection panel
    int x = ofGetWidth() - 250;
    int y = 150;
    int width = 240;
    int lineHeight = 20;
    
    // Background
    ofSetColor(0, 0, 0, 180);
    ofDrawRectangle(x, y, width, 250);
    
    // Title
    ofSetColor(255, 255, 0);
    ofDrawBitmapString("Audio Input Devices", x + 10, y + 20);
    ofDrawBitmapString("Press Shift+D to cycle", x + 10, y + 35);
    
    // Get device list
    auto deviceList = audioManager->getAudioDeviceList();
    int currentIndex = audioManager->getCurrentDeviceIndex();
    
    // Draw device list
    ofSetColor(200, 200, 200);
    int startY = y + 55;
    
    for (int i = 0; i < deviceList.size(); i++) {
        // Highlight current device
        if (i == currentIndex) {
            ofSetColor(100, 200, 100);
            ofDrawRectangle(x + 5, startY + i * lineHeight - 12, width - 10, lineHeight);
            ofSetColor(0);
        } else {
            ofSetColor(200, 200, 200);
        }
        
        ofDrawBitmapString(ofToString(i) + ": " + deviceList[i], x + 10, startY + i * lineHeight);
    }
    
    // Input level meter
    ofSetColor(255, 255, 255);
    ofDrawBitmapString("Input Level", x + 10, startY + deviceList.size() * lineHeight + 20);
    
    // Draw meter background
    ofSetColor(50, 50, 50);
    ofDrawRectangle(x + 10, startY + deviceList.size() * lineHeight + 25, width - 20, 15);
    
    // Draw meter fill based on input level (implement the actual level)
    float inputLevel = 0.0f; // Get from your AudioReactivityManager
    ofSetColor(0, 200, 0);
    ofDrawRectangle(x + 10, startY + deviceList.size() * lineHeight + 25,
                   (width - 20) * inputLevel, 15);
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawAudioVisualization() {
    if (!paramManager->isDebugEnabled() || !audioManager->isEnabled()) return;
    
    // Draw visualization at the bottom of the screen
    ofPushStyle();
    
    int x = 10;
    int y = ofGetHeight() - 180; // A bit higher for more space
    int width = 400;  // Wider for more detail
    int height = 160; // Taller for better visibility
    
    // Draw background
    ofSetColor(0, 0, 0, 200);
    ofDrawRectangle(x, y, width, height);
    
    // Draw title
    ofSetColor(255, 255, 0);
    ofDrawBitmapString("Audio Reactivity (ofxFft)", x + 10, y + 15);
    
    // Draw frequency bands
    auto bands = audioManager->getAllBands();
    int numBands = audioManager->getNumBands();
    
    // Draw band labels with descriptions
    ofSetColor(200, 200, 200);
    ofDrawBitmapString("Frequency Bands:", x + 10, y + 35);
    
    // Band descriptions
    std::vector<std::string> bandDesc = {
        "Sub bass (20-60Hz)",
        "Bass (60-250Hz)",
        "Low mids (250-500Hz)",
        "Mids (500-2000Hz)",
        "High mids (2-4kHz)",
        "Presence (4-6kHz)",
        "Brilliance (6-12kHz)",
        "Air (12-20kHz)"
    };
    
    // Calculate bar width based on number of bands
    float barWidth = (width - 20) / numBands;
    
    for (int i = 0; i < numBands; i++) {
        float bandValue = bands[i];
        
        // Draw band number and label
        ofSetColor(200, 200, 200);
        ofDrawBitmapString(ofToString(i), x + 10 + i * barWidth + barWidth/2 - 3, y + height - 15);
        
        // Draw vertical bar
        float barHeight = ofMap(bandValue, 0, 1, 0, height - 80);
        
        // Color based on band intensity
        ofColor barColor;
        barColor.setHsb(ofMap(i, 0, numBands, 0, 255), 200, 255);
        ofSetColor(barColor);
        
        ofDrawRectangle(
            x + 10 + i * barWidth,
            y + height - 35 - barHeight,
            barWidth - 2,
            barHeight
        );
        
        // Draw peak indicator
        ofSetColor(255);
        ofDrawLine(
            x + 10 + i * barWidth,
            y + height - 35 - barHeight - 2,
            x + 10 + i * barWidth + barWidth - 2,
            y + height - 35 - barHeight - 2
        );
    }
    
    // Draw input level meter
    float inputLevel = audioManager->getAudioInputLevel();
    
    // Level meter background
    ofSetColor(50, 50, 50);
    ofDrawRectangle(x + 10, y + 50, width - 20, 10);
    
    // Level meter fill
    ofSetColor(0, 200, 0);
    if (inputLevel > 0.7) ofSetColor(255, 0, 0); // Red when clipping
    else if (inputLevel > 0.5) ofSetColor(255, 200, 0); // Yellow when high
    
    ofDrawRectangle(x + 10, y + 50, (width - 20) * inputLevel, 10);
    
    // Draw level text
    ofSetColor(255);
    ofDrawBitmapString(
        "Input Level: " + ofToString(inputLevel, 2),
        x + 10,
        y + 80
    );
    
    // Draw sensitivity
    ofDrawBitmapString(
        "Sensitivity: " + ofToString(audioManager->getSensitivity(), 2),
        x + 10,
        y + 95
    );
    
    // Draw smoothing
    ofDrawBitmapString(
        "Smoothing: " + ofToString(audioManager->getSmoothing(), 2),
        x + 10,
        y + 110
    );
    
    // Draw active mappings count
    ofDrawBitmapString(
        "Mappings: " + ofToString(audioManager->getMappings().size()),
        x + 10,
        y + 125
    );
    
    // Draw key controls hints
    ofSetColor(180, 180, 255);
    ofDrawBitmapString("Press Shift+[ or Shift+] to adjust smoothing", x + 10, y + 155);
    ofDrawBitmapString("Press Shift+- or Shift+= to adjust sensitivity", x + 10, y + 170);
    ofDrawBitmapString("Press Shift+N to toggle normalization", x + 10, y + 185);

    ofPopStyle();
    
    // Debug: Print out raw band values
//    std::stringstream bandValues;
//    bandValues << "Band Values: ";
//    for (int i = 0; i < numBands; i++) {
//        bandValues << bands[i] << " ";
//    }
//    ofLogNotice("AudioVisualization") << bandValues.str();

}

//--------------------------------------------------------------
void ofApp::drawDebugInfo() {
    if (!paramManager->isDebugEnabled()) {
        return;
    }
    
    ofPushStyle();
    ofSetColor(255, 255, 0); // Yellow text for visibility
    
    // Position for drawing text
    int x = 10;
    int y = 20;
    int lineHeight = 15;
    
    // Add performance info at the top
    bool performanceMode = paramManager->isPerformanceModeEnabled();
    ofSetColor(performanceMode ? ofColor::red : ofColor::green);
    ofDrawBitmapString("Performance Mode: " + string(performanceMode ? "ON" : "OFF"), x, y);
    y += lineHeight;
    
    ofSetColor(255, 255, 0); // Back to yellow text
    
    // Add performance graph
    y += lineHeight;
    ofDrawBitmapString("--- Performance ---", x, y);
    y += lineHeight;

    float avgFrameTime = 0;
    for (int i = 0; i < 60; i++) {
        avgFrameTime += performanceMetrics[i];
    }
    avgFrameTime /= 60.0f;
    float avgFps = 1.0f / (avgFrameTime > 0 ? avgFrameTime : 0.0001f);
    
    ofDrawBitmapString("Avg frame time: " + ofToString(avgFrameTime * 1000, 2) + "ms", x, y);
    y += lineHeight;
    ofDrawBitmapString("Avg FPS: " + ofToString(avgFps, 1), x, y);
    y += lineHeight;

    // Draw graph
    int graphWidth = 200;
    int graphHeight = 40;
    ofDrawRectangle(x, y, graphWidth, graphHeight);
    ofSetColor(0);
    ofDrawRectangle(x+1, y+1, graphWidth-2, graphHeight-2);
    
    // Use different colors for performance mode
    if (performanceMode) {
        ofSetColor(255, 128, 0); // Orange in performance mode
    } else {
        ofSetColor(0, 255, 0); // Normal green
    }
    
    for (int i = 0; i < 60; i++) {
        float h = ofMap(performanceMetrics[i], 0, 1.0f/30.0f, 0, graphHeight-4);
        ofDrawLine(x + 2 + i * 3, y + graphHeight - 2, x + 2 + i * 3, y + graphHeight - 2 - h);
    }
    ofSetColor(255, 255, 0);
    
    // App info
    y += graphHeight + lineHeight;
    ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), x, y);
    y += lineHeight;
    
    // Performance settings
    if (performanceMode) {
        ofDrawBitmapString("--- Performance Settings ---", x, y);
        y += lineHeight;
        ofDrawBitmapString("Target scale: " + ofToString(paramManager->getPerformanceScale()), x, y);
        y += lineHeight;
        ofDrawBitmapString("Noise update interval: " + ofToString(paramManager->getNoiseUpdateInterval()), x, y);
        y += lineHeight;
        ofDrawBitmapString("High quality: " + ofToString(paramManager->isHighQualityEnabled() ? "Yes" : "No"), x, y);
        y += lineHeight;
    }
    
    // Current mesh info
    ofDrawBitmapString("Mesh type: " + ofToString(static_cast<int>(paramManager->getMeshType())), x, y);
    y += lineHeight;
    ofDrawBitmapString("Resolution: " + ofToString(paramManager->getScale()), x, y);
    y += lineHeight;
    
    // Parameter values
    ofDrawBitmapString("--- Parameters ---", x, y);
    y += lineHeight;
    ofDrawBitmapString("Luma key: " + ofToString(paramManager->getLumaKeyLevel(), 3), x, y);
    y += lineHeight;
    ofDrawBitmapString("X Displace: " + ofToString(paramManager->getXDisplace(), 3), x, y);
    y += lineHeight;
    ofDrawBitmapString("Y Displace: " + ofToString(paramManager->getYDisplace(), 3), x, y);
    y += lineHeight;
    ofDrawBitmapString("Z Freq: " + ofToString(paramManager->getZFrequency(), 3), x, y);
    y += lineHeight;
    ofDrawBitmapString("X Freq: " + ofToString(paramManager->getXFrequency(), 3), x, y);
    y += lineHeight;
    ofDrawBitmapString("Y Freq: " + ofToString(paramManager->getYFrequency(), 3), x, y);
    y += lineHeight;
    
    // Toggle states
    ofDrawBitmapString("--- Toggles ---", x, y);
    y += lineHeight;
    ofDrawBitmapString("Wireframe: " + ofToString(paramManager->isWireframeEnabled()), x, y);
    y += lineHeight;
    ofDrawBitmapString("Luma Key: " + ofToString(paramManager->isLumaKeyEnabled()), x, y);
    y += lineHeight;
    ofDrawBitmapString("Invert: " + ofToString(paramManager->isInvertEnabled()), x, y);
    y += lineHeight;
    ofDrawBitmapString("B&W: " + ofToString(paramManager->isBlackAndWhiteEnabled()), x, y);
    y += lineHeight;
    
    // Shader debug section
    y += lineHeight * 2;
    ofDrawBitmapString("--- Shader Debug ---", x, y);
    y += lineHeight;
    
    // Shader uniform values
    ofDrawBitmapString("Shader active: " + std::string(shaderDisplace.isLoaded() ? "YES" : "NO"), x, y);
    y += lineHeight;
    
    // Basic texture info
    ofDrawBitmapString("Camera texture: " + std::string(camera.isInitialized() ? "OK" : "Not initialized"), x, y);
    y += lineHeight;
    
    ofDrawBitmapString("X Noise texture: " + std::string(xNoiseImage.isAllocated() ? "OK" : "Not allocated"), x, y);
    y += lineHeight;
    
    ofDrawBitmapString("Y Noise texture: " + std::string(yNoiseImage.isAllocated() ? "OK" : "Not allocated"), x, y);
    y += lineHeight;
    
    ofDrawBitmapString("Z Noise texture: " + std::string(zNoiseImage.isAllocated() ? "OK" : "Not allocated"), x, y);
    y += lineHeight;
    
    // FBO format info
    ofDrawBitmapString("Camera FBO format: " + ofToString(cameraFbo.getTexture().getTextureData().glInternalFormat), x, y);
    y += lineHeight;
    
    ofDrawBitmapString("Aspect FBO format: " + ofToString(aspectFixFbo.getTexture().getTextureData().glInternalFormat), x, y);
    y += lineHeight;
    
    ofDrawBitmapString("Main FBO format: " + ofToString(mainFbo.getTexture().getTextureData().glInternalFormat), x, y);
    y += lineHeight;
    
    // Video device info
    y += lineHeight * 2;
    ofDrawBitmapString("--- Video Settings ---", x, y);
    y += lineHeight;
    ofDrawBitmapString("Device: " + paramManager->getVideoDevicePath(), x, y);
    y += lineHeight;
    ofDrawBitmapString("Format: " + paramManager->getVideoFormat(), x, y);
    y += lineHeight;
    ofDrawBitmapString("Size: " + ofToString(paramManager->getVideoWidth()) + "x"
                      + ofToString(paramManager->getVideoHeight()), x, y);
    y += lineHeight;
    ofDrawBitmapString("FPS: " + ofToString(paramManager->getVideoFrameRate()), x, y);
    y += lineHeight;

    // Check if camera is initialized and working
    ofDrawBitmapString("Camera initialized: " + std::string(camera.isInitialized() ? "YES" : "NO"), x, y);
    y += lineHeight;
    ofDrawBitmapString("Last frame new: " + std::string(camera.isFrameNew() ? "YES" : "NO"), x, y);
    y += lineHeight;
    
    // MIDI info on the right side
    int rightX = ofGetWidth() - 300;
    y = 20;
    
    ofDrawBitmapString("--- MIDI Info ---", rightX, y);
    y += lineHeight;
    ofDrawBitmapString("Device: " + midiManager->getCurrentDeviceName(), rightX, y);
    y += lineHeight;
    
    // Recent MIDI messages
    ofDrawBitmapString("Recent messages:", rightX, y);
    y += lineHeight;
    
    auto midiMessages = midiManager->getRecentMessages();
    int maxMessages = std::min(static_cast<int>(midiMessages.size()), 10);
    
    if (midiMessages.size() > 0) {
        for (int i = midiMessages.size() - maxMessages; i < midiMessages.size(); i++) {
            const auto& msg = midiMessages[i];
            std::string msgType;
            
            switch (msg.status) {
                case MIDI_NOTE_ON: msgType = "Note On"; break;
                case MIDI_NOTE_OFF: msgType = "Note Off"; break;
                case MIDI_CONTROL_CHANGE: msgType = "CC"; break;
                case MIDI_PROGRAM_CHANGE: msgType = "Program"; break;
                case MIDI_PITCH_BEND: msgType = "Pitch Bend"; break;
                case MIDI_AFTERTOUCH: msgType = "Aftertouch"; break;
                case MIDI_POLY_AFTERTOUCH: msgType = "Poly AT"; break;
                default: msgType = "Other"; break;
            }
            
            std::string msgInfo = msgType + " Ch:" + ofToString(msg.channel) +
                                 " Ctrl:" + ofToString(msg.control) +
                                 " Val:" + ofToString(msg.value);
            
            ofDrawBitmapString(msgInfo, rightX, y);
            y += lineHeight;
        }
    } else {
        ofDrawBitmapString("No MIDI messages received", rightX, y);
        y += lineHeight;
    }
    
    // Show MIDI mappings in debug view if enabled
    bool showMidiMappings = true; // Change to a class member later
    if (showMidiMappings) {
        // Draw in a separate column
        int mapX = 400;
        int mapY = 20;
        
        ofDrawBitmapString("--- MIDI Mappings ---", mapX, mapY);
        mapY += lineHeight;
        
        // Instead of using getMidiMappings(), access the public vector directly
        const auto& mappings = paramManager->midiMappings;
        for (const auto& mapping : mappings) {
            std::string mappingInfo =
                "CC" + ofToString(mapping.controlNumber) +
                " Ch" + ofToString(mapping.channel) +
                " -> " + mapping.paramId +
                " (" + ofToString(mapping.minValue) + " to " +
                ofToString(mapping.maxValue) + ")";
            
            ofDrawBitmapString(mappingInfo, mapX, mapY);
            mapY += lineHeight;
        }
    }
    
    // P-Lock system status
    y += lineHeight;
    ofDrawBitmapString("Recording: " + std::string(paramManager->isRecordingEnabled() ? "ON" : "OFF"), rightX, y);
    y += lineHeight;
    ofSetColor(180, 180, 255);
    
    ofDrawBitmapString("Press Shift+P to toggle performance mode", x, ofGetHeight() - 30);
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    // Handle keyboard controls for parameters
    if (!paramManager || !audioManager || !meshGenerator) {
        ofLogError("ofApp") << "Critical subsystems not initialized!";
        return;
    }
    
    switch (key) {
        // Luma key level
        case 'a':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                // Toggle audio reactivity with Shift+A
                audioManager->setEnabled(!audioManager->isEnabled());
                ofLogNotice("ofApp") << "Audio reactivity: " << (audioManager->isEnabled() ? "ON" : "OFF");
            } else {
                // Regular 'a' key functionality
                keyControls.lumaKey += 0.01f;
            }
            break;
            
        case 'z': keyControls.lumaKey -= 0.01f; break;
        
        // Z Frequency
        case 's': keyControls.zFrequency += 0.0001f; break;
        case 'x': keyControls.zFrequency -= 0.0001f; break;
        
        // Z LFO arg
        case 'd': keyControls.zLfoArg += 0.001f; break;
        case 'c': keyControls.zLfoArg -= 0.001f; break;
        
        // Z LFO amplitude
        case 'f': keyControls.zLfoAmp += 0.001f; break;
        case 'v': keyControls.zLfoAmp -= 0.001f; break;
        
        // X Frequency
        case 'g': keyControls.xFrequency += 0.001f; break;
        case 'b': keyControls.xFrequency -= 0.001f; break;
        
        // X LFO arg
        case 'h': keyControls.xLfoArg += 0.001f; break;
        case 'n': keyControls.xLfoArg -= 0.001f; break;
        
        // X LFO amplitude
        case 'j': keyControls.xLfoAmp += 0.1f; break;
        case 'm': keyControls.xLfoAmp -= 0.1f; break;
        
        // Y Frequency
        case 'k': keyControls.yFrequency += 0.001f; break;
        case ',': keyControls.yFrequency -= 0.001f; break;
        
        // Y LFO arg
        case 'l': keyControls.yLfoArg += 0.001f; break;
        case '.': keyControls.yLfoArg -= 0.001f; break;
        
        // Y LFO amplitude
        case ';': keyControls.yLfoAmp += 0.1f; break;
        case '/': keyControls.yLfoAmp -= 0.1f; break;
        
        // Center X
        case 't': keyControls.centerX += 5.0f; break;
        case 'y': keyControls.centerX -= 5.0f; break;
        
        // Center Y
        case 'u': keyControls.centerY += 5.0f; break;
        case 'i': keyControls.centerY -= 5.0f; break;
        
        // Zoom
        case 'o': keyControls.zoom += 5.0f; break;
        case 'p': keyControls.zoom -= 5.0f; break;
        
        // Y Displacement
        case 'e': keyControls.yDisplace += 0.01f; break;
        case 'r': keyControls.yDisplace -= 0.01f; break;
        
        // X Displacement
        case 'q': keyControls.xDisplace += 0.01f; break;
        case 'w': keyControls.xDisplace -= 0.01f; break;
        
        // Scale
        case ']':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                // Shift+] increases audio smoothing
                audioManager->setSmoothing(audioManager->getSmoothing() + 0.05f);
                ofLogNotice("ofApp") << "Audio smoothing: " << audioManager->getSmoothing();
            } else {
                // Normal ] increases scale
                int scale = paramManager->getScale() + 1;
                paramManager->setScale(scale);
                meshGenerator->updateMeshFromParameters();
            }
            break;
            
        case '[':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                // Shift+[ decreases audio smoothing
                audioManager->setSmoothing(audioManager->getSmoothing() - 0.05f);
                ofLogNotice("ofApp") << "Audio smoothing: " << audioManager->getSmoothing();
            } else {
                // Normal [ decreases scale
                int scale = paramManager->getScale() - 1;
                if (scale < 1) scale = 1;
                paramManager->setScale(scale);
                meshGenerator->updateMeshFromParameters();
            }
            break;
        
        // Toggle switches
        case '1': paramManager->setLumaKeyEnabled(!paramManager->isLumaKeyEnabled()); break;
        case '2': paramManager->setBrightEnabled(!paramManager->isBrightEnabled()); break;
        case '3': paramManager->setInvertEnabled(!paramManager->isInvertEnabled()); break;
        case '5': paramManager->setBlackAndWhiteEnabled(!paramManager->isBlackAndWhiteEnabled()); break;
        
        // LFO shapes
        case '6': {
            int shape = static_cast<int>(paramManager->getZLfoShape()) + 1;
            paramManager->setZLfoShape(static_cast<LfoShape>(shape % 4));
            break;
        }
        case '7': {
            int shape = static_cast<int>(paramManager->getXLfoShape()) + 1;
            paramManager->setXLfoShape(static_cast<LfoShape>(shape % 4));
            break;
        }
        case '8': {
            int shape = static_cast<int>(paramManager->getYLfoShape()) + 1;
            paramManager->setYLfoShape(static_cast<LfoShape>(shape % 4));
            break;
        }
        
        // Mesh types
        case '9':
            paramManager->setMeshType(MeshType::VerticalLines);
            meshGenerator->updateMeshFromParameters();
            break;
        case '0':
            paramManager->setMeshType(MeshType::HorizontalLines);
            meshGenerator->updateMeshFromParameters();
            break;
        
        // Minus key - Handles multiple combinations
        case '-':
            if (ofGetKeyPressed(OF_KEY_SHIFT) && ofGetKeyPressed(OF_KEY_CONTROL)) {
                // Decrease frame rate with Shift+Ctrl+minus
                int fps = paramManager->getVideoFrameRate() - 5;
                if (fps < 5) fps = 5;
                paramManager->setVideoFrameRate(fps);
                
                // Reinitialize camera to apply - only if camera is initialized
                if (camera.isInitialized()) {
                    camera.close();
                    setupCamera(width, height);
                }
                
                ofLogNotice("ofApp") << "Set frame rate to: " << fps;
            }
            else if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                // Shift+Minus decreases audio sensitivity
                audioManager->setSensitivity(audioManager->getSensitivity() - 0.1f);
                ofLogNotice("ofApp") << "Audio sensitivity: " << audioManager->getSensitivity();
            }
            else {
                // Regular mesh control
                paramManager->setMeshType(MeshType::TriangleGrid);
                paramManager->setWireframeEnabled(false);
                meshGenerator->updateMeshFromParameters();
            }
            break;
            
        // Equals key - Handles multiple combinations
        case '=':
            if (ofGetKeyPressed(OF_KEY_SHIFT) && ofGetKeyPressed(OF_KEY_CONTROL)) {
                // Increase frame rate with Shift+Ctrl+equal
                int fps = paramManager->getVideoFrameRate() + 5;
                if (fps > 60) fps = 60;
                paramManager->setVideoFrameRate(fps);
                
                // Reinitialize camera to apply - only if camera is initialized
                if (camera.isInitialized()) {
                    camera.close();
                    setupCamera(width, height);
                }
                
                ofLogNotice("ofApp") << "Set frame rate to: " << fps;
            }
            else if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                // Shift+Plus increases audio sensitivity
                audioManager->setSensitivity(audioManager->getSensitivity() + 0.1f);
                ofLogNotice("ofApp") << "Audio sensitivity: " << audioManager->getSensitivity();
            }
            else {
                // Regular mesh control
                paramManager->setMeshType(MeshType::TriangleGrid);
                paramManager->setWireframeEnabled(true);
                meshGenerator->updateMeshFromParameters();
            }
            break;
        
        // Modulation toggles
        case '!': paramManager->setZRingModEnabled(!paramManager->isZRingModEnabled()); break;
        case '@': paramManager->setXRingModEnabled(!paramManager->isXRingModEnabled()); break;
        case '#': paramManager->setYRingModEnabled(!paramManager->isYRingModEnabled()); break;
        case '$': paramManager->setZPhaseModEnabled(!paramManager->isZPhaseModEnabled()); break;
        case '%': paramManager->setXPhaseModEnabled(!paramManager->isXPhaseModEnabled()); break;
        case '^': paramManager->setYPhaseModEnabled(!paramManager->isYPhaseModEnabled()); break;
            
        // Debug toggle
        case '`':  // Backtick key for debug toggle
            paramManager->setDebugEnabled(!paramManager->isDebugEnabled());
            break;
            
        case 'P':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                // Toggle performance mode
                bool performanceMode = !paramManager->isPerformanceModeEnabled();
                paramManager->setPerformanceModeEnabled(performanceMode);
                
                if (performanceMode) {
                    // Apply performance optimizations immediately
                    // Reduce target framerate
                    ofSetFrameRate(24);
                    
                    // Force mesh update with performance scale
                    int currentScale = paramManager->getScale();
                    int performanceScale = paramManager->getPerformanceScale();
                    
                    // Only reduce scale if current scale is larger than performance scale
                    if (currentScale > performanceScale) {
                        paramManager->setScale(performanceScale);
                        meshGenerator->updateMeshFromParameters();
                    }
                } else {
                    // Restore normal settings
                    ofSetFrameRate(30);
                }
                
                ofLogNotice("ofApp") << "Performance mode: " << (performanceMode ? "ON" : "OFF");
            }
            break;
            
        case 'R':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                // Reset parameters (original Shift+R functionality)
                paramManager->resetToDefaults();
                keyControls = KeyControls(); // Reset all key controls
                ofLogNotice("ofApp") << "All parameters reset to defaults";
            }
            break;
            
        // Midi Mapping
        case 'M':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                paramManager->generateMappingTemplate(ofToDataPath("mapping_template.xml"));
            }
            break;
        
        // Video device cycling
        case 'V':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                try {
                    // Reload device list
                    listVideoDevices();
                    
                    // Safety check to avoid array out of bounds
                    if (!videoDevices.empty()) {
                        int nextDeviceID = (paramManager->getVideoDeviceID() + 1) % videoDevices.size();
                        selectVideoDevice(nextDeviceID);
                        ofLogNotice("ofApp") << "Cycled to video device: " << paramManager->getVideoDevicePath();
                    } else {
                        ofLogWarning("ofApp") << "No video devices available to cycle through";
                    }
                } catch (const std::exception& e) {
                    ofLogError("ofApp") << "Exception during video device cycling: " << e.what();
                }
            }
            break;

        // Video format cycling
        case 'F':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                try {
                    if (!videoFormats.empty()) {
                        cycleVideoFormat();
                        ofLogNotice("ofApp") << "Cycled to format: " << paramManager->getVideoFormat();
                    } else {
                        ofLogWarning("ofApp") << "No video formats available to cycle through";
                    }
                } catch (const std::exception& e) {
                    ofLogError("ofApp") << "Exception during format cycling: " << e.what();
                }
            }
            break;

        case 'Z':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                try {
                    // Get current video device and format
                    std::string devicePath = paramManager->getVideoDevicePath();
                    uint32_t formatCode = V4L2Helper::formatNameToCode(paramManager->getVideoFormat());
                    
                    // Get resolutions for this format
                    videoResolutions = V4L2Helper::listResolutions(devicePath, formatCode);
                    
                    if (!videoResolutions.empty()) {
                        // Increment to next resolution
                        currentResolutionIndex = (currentResolutionIndex + 1) % videoResolutions.size();
                        
                        // Get the selected resolution
                        int newWidth = videoResolutions[currentResolutionIndex].width;
                        int newHeight = videoResolutions[currentResolutionIndex].height;
                        
                        // Set the resolution
                        setVideoFormat(paramManager->getVideoFormat(), newWidth, newHeight);
                        
                        ofLogNotice("ofApp") << "Cycled to resolution: "
                                          << newWidth << "x" << newHeight;
                    } else {
                        ofLogWarning("ofApp") << "No resolutions available to cycle through";
                    }
                } catch (const std::exception& e) {
                    ofLogError("ofApp") << "Exception during resolution cycling: " << e.what();
                }
            }
            break;

        // Toggle video debug panel (using 'I' instead of 'R' which is already used)
        case 'I':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                try {
                    // List video devices and print verbose debug info
                    listVideoDevices();
                    
                    // Print current settings
                    ofLogNotice("ofApp") << "Current video settings:";
                    ofLogNotice("ofApp") << "  Device: " << paramManager->getVideoDevicePath();
                    ofLogNotice("ofApp") << "  Format: " << paramManager->getVideoFormat();
                    ofLogNotice("ofApp") << "  Size: " << paramManager->getVideoWidth()
                                       << "x" << paramManager->getVideoHeight();
                    ofLogNotice("ofApp") << "  FPS: " << paramManager->getVideoFrameRate();
                } catch (const std::exception& e) {
                    ofLogError("ofApp") << "Exception during video info display: " << e.what();
                }
            }
            break;

        // Audio device cycling
        case 'D':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                auto deviceList = audioManager->getAudioDeviceList();
                if (!deviceList.empty()) {
                    int currentIndex = audioManager->getCurrentDeviceIndex();
                    int nextIndex = (currentIndex + 1) % deviceList.size();
                    audioManager->selectAudioDevice(nextIndex);
                    ofLogNotice("ofApp") << "Selected audio device: " << audioManager->getCurrentDeviceName();
                } else {
                    ofLogWarning("ofApp") << "No audio devices available to cycle through";
                }
            }
            break;
        
        case 'N':
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                // Toggle FFT normalization with Shift+N
                bool normalizationEnabled = audioManager->isNormalizationEnabled();
                audioManager->setNormalizationEnabled(!normalizationEnabled);
                ofLogNotice("ofApp") << "FFT normalization: " << (audioManager->isNormalizationEnabled() ? "ON" : "OFF");
            }
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    // Handle key release events if needed
}

//--------------------------------------------------------------
void ofApp::setupAudioReactivity() {
    // Enable audio processing
    audioManager->setEnabled(true);
    
    // More aggressive sensitivity and less smoothing
    audioManager->setSensitivity(2.0f);    // Increased sensitivity
    audioManager->setSmoothing(0.8f);       // Less smoothing for faster response
    
    // Clear any existing mappings
    audioManager->clearMappings();
    
    // More pronounced mappings
    AudioReactivityManager::BandMapping xMapping;
    xMapping.band = 6;                // Brilliance band
    xMapping.paramId = "x_displace";
    xMapping.scale = 2.0f;            // Higher scale for more dramatic effect
    xMapping.min = -0.5f;             // Wider range
    xMapping.max = 0.5f;
    xMapping.additive = false;
    audioManager->addMapping(xMapping);
    
    // Similar adjustments for other mappings...
    AudioReactivityManager::BandMapping yMapping;
    yMapping.band = 3;
    yMapping.paramId = "y_displace";
    yMapping.scale = 2.0f;
    yMapping.min = -0.5f;
    yMapping.max = 0.5f;
    yMapping.additive = false;
    audioManager->addMapping(yMapping);
    
    // More dramatic zoom mapping
    AudioReactivityManager::BandMapping zoomMapping;
    zoomMapping.band = 1;             // Bass band
    zoomMapping.paramId = "zoom";
    zoomMapping.scale = 1.5f;         // More intense zoom
    zoomMapping.min = 0.0f;
    zoomMapping.max = 1.0f;           // Wider zoom range
    zoomMapping.additive = false;
    audioManager->addMapping(zoomMapping);
        
    // Map sub bass (band 0 - Sub bass 20-60Hz) to affect z_frequency
    AudioReactivityManager::BandMapping zFreqMapping;
    zFreqMapping.band = 0;            // Sub bass band
    zFreqMapping.paramId = "z_frequency"; // Parameter ID
    zFreqMapping.scale = 1.0f;        // Full intensity
    zFreqMapping.min = 0.02f;         // Minimum frequency
    zFreqMapping.max = 0.1f;          // Maximum frequency
    zFreqMapping.additive = false;    // Use direct mode
    audioManager->addMapping(zFreqMapping);
    
    // Add mapping for high mids (band 4 - High mids 2-4kHz) to x_lfo_amp
    AudioReactivityManager::BandMapping xLfoAmpMapping;
    xLfoAmpMapping.band = 4;          // High mids band
    xLfoAmpMapping.paramId = "x_lfo_amp"; // Parameter ID
    xLfoAmpMapping.scale = 1.0f;      // Full intensity
    xLfoAmpMapping.min = 0.0f;        // Minimum amplitude
    xLfoAmpMapping.max = 0.8f;        // Maximum amplitude
    xLfoAmpMapping.additive = false;  // Use direct mode
    audioManager->addMapping(xLfoAmpMapping);
    
    // Try to select the built-in microphone
    auto devices = audioManager->getAudioDeviceList();
    if (!devices.empty()) {
        // Look for built-in microphone or use first available device
        int deviceIndex = 0;
        for (int i = 0; i < devices.size(); i++) {
            std::string deviceName = devices[i];
            std::transform(deviceName.begin(), deviceName.end(), deviceName.begin(), ::tolower);
            if (deviceName.find("built-in") != std::string::npos ||
                deviceName.find("microphone") != std::string::npos) {
                deviceIndex = i;
                break;
            }
        }
        audioManager->selectAudioDevice(deviceIndex);
        ofLogNotice("ofApp") << "Selected audio device: " << audioManager->getCurrentDeviceName();
    }
    
    // Reset displacement values to avoid residual effects
    paramManager->setXDisplace(0.0f);
    paramManager->setYDisplace(0.0f);
    
    ofLogNotice("ofApp") << "Audio reactivity set up with " << audioManager->getMappings().size() << " mappings";
}


void ofApp::listVideoDevices() {
    // Get V4L2 devices using the platform-independent helper
    videoDevices = V4L2Helper::listDevices();
    
    // Print devices to console
    ofLogNotice("ofApp") << "Available video devices (" << videoDevices.size() << "):";
    for (size_t i = 0; i < videoDevices.size(); i++) {
        const auto& device = videoDevices[i];
        ofLogNotice("ofApp") << i << ": " << device.path << " - " << device.name;
    }
    
    // Try to get formats for the current device
    std::string devicePath = paramManager->getVideoDevicePath();
    if (devicePath.empty() && !videoDevices.empty()) {
        devicePath = videoDevices[0].path;
    }
    
    // Get formats for the current device
    videoFormats = V4L2Helper::listFormats(devicePath);
    
    // Print formats to console
    ofLogNotice("ofApp") << "Available formats for " << devicePath << " (" << videoFormats.size() << "):";
    for (size_t i = 0; i < videoFormats.size(); i++) {
        const auto& format = videoFormats[i];
        ofLogNotice("ofApp") << i << ": " << format.name << " (" << format.fourcc << ")";
        
        // Get resolutions for this format
        auto resolutions = V4L2Helper::listResolutions(devicePath, format.pixelFormat);
        ofLogNotice("ofApp") << "  Available resolutions (" << resolutions.size() << "):";
        for (size_t j = 0; j < resolutions.size(); j++) {
            ofLogNotice("ofApp") << "  " << j << ": " << resolutions[j].width << "x" << resolutions[j].height;
        }
    }
}

bool ofApp::selectVideoDevice(int deviceIndex) {
    // Make sure we have a list of devices first
    if (videoDevices.empty()) {
        listVideoDevices();
    }
    
    if (deviceIndex < 0 || deviceIndex >= videoDevices.size()) {
        ofLogError("ofApp") << "Invalid device index: " << deviceIndex;
        return false;
    }
    
    // Close current camera
    if (camera.isInitialized()) {
        camera.close();
    }
    
    // Set new device info
    std::string devicePath = videoDevices[deviceIndex].path;
    paramManager->setVideoDevicePath(devicePath);
    paramManager->setVideoDeviceID(deviceIndex);
    
    ofLogNotice("ofApp") << "Selected video device " << deviceIndex << ": " << devicePath;
    
    // Update formats for the new device
    videoFormats = V4L2Helper::listFormats(devicePath);
    
    // Try to select the first available format
    if (!videoFormats.empty()) {
        // Get current format or use first format
        V4L2Helper::VideoFormat currentFormat = V4L2Helper::getCurrentFormat(devicePath);
        
        // Find matching format in our list or use first
        currentFormatIndex = 0;
        for (size_t i = 0; i < videoFormats.size(); i++) {
            if (videoFormats[i].pixelFormat == currentFormat.pixelFormat) {
                currentFormatIndex = i;
                break;
            }
        }
        
        // Set the format in the parameter manager
        paramManager->setVideoFormat(videoFormats[currentFormatIndex].fourcc);
    }
    
    // Reinitialize camera
    setupCamera(width, height);
    
    return true;
}

bool ofApp::selectVideoDevice(const std::string& devicePath) {
    for (size_t i = 0; i < videoDevices.size(); i++) {
        if (videoDevices[i].path == devicePath) {
            return selectVideoDevice(i);
        }
    }
    
    ofLogError("ofApp") << "Device not found: " << devicePath;
    return false;
}

bool ofApp::setVideoFormat(const std::string& format, int width, int height) {
    std::string devicePath = paramManager->getVideoDevicePath();
    
    // Convert format string to V4L2 pixel format code
    uint32_t formatCode = V4L2Helper::formatNameToCode(format);
    
    // Try to set the format
    if (V4L2Helper::setFormat(devicePath, formatCode, width, height)) {
        // Update parameter manager
        paramManager->setVideoFormat(format);
        paramManager->setVideoWidth(width);
        paramManager->setVideoHeight(height);
        
        // Get the actual format that was set
        V4L2Helper::VideoFormat currentFormat = V4L2Helper::getCurrentFormat(devicePath);
        
        ofLogNotice("ofApp") << "Set video format: " << currentFormat.name
                           << " (" << currentFormat.fourcc << ") "
                           << width << "x" << height;
        
        // Reinitialize camera
        camera.close();
        setupCamera(width, height);
        
        return true;
    }
    
    return false;
}

void ofApp::cycleVideoFormat() {
    if (videoFormats.empty()) {
        ofLogWarning("ofApp") << "No formats available to cycle";
        return;
    }
    
    // Increment format index
    currentFormatIndex = (currentFormatIndex + 1) % videoFormats.size();
    
    // Get the selected format
    const auto& format = videoFormats[currentFormatIndex];
    
    // Get resolutions for this format
    std::string devicePath = paramManager->getVideoDevicePath();
    videoResolutions = V4L2Helper::listResolutions(devicePath, format.pixelFormat);
    
    // Use first resolution or current one
    int width = paramManager->getVideoWidth();
    int height = paramManager->getVideoHeight();
    
    if (!videoResolutions.empty()) {
        width = videoResolutions[0].width;
        height = videoResolutions[0].height;
    }
    
    // Set the new format
    setVideoFormat(format.fourcc, width, height);
}

void ofApp::drawVideoDevicesUI() {
    if (!paramManager->isDebugEnabled()) return;
    
    ofPushStyle();
    
    // Position and size for the device selection panel
    int x = ofGetWidth() - 250;
    int y = 410;
    int width = 240;
    int height = 250;
    int lineHeight = 20;
    
    // Background
    ofSetColor(0, 0, 0, 180);
    ofDrawRectangle(x, y, width, height);
    
    // Title
    ofSetColor(255, 255, 0);
    ofDrawBitmapString("Video Device Settings", x + 10, y + 20);
    
    // Get current settings
    std::string devicePath = paramManager->getVideoDevicePath();
    int deviceID = paramManager->getVideoDeviceID();
    std::string format = paramManager->getVideoFormat();
    int vidWidth = paramManager->getVideoWidth();
    int vidHeight = paramManager->getVideoHeight();
    int frameRate = paramManager->getVideoFrameRate();
    
    // Draw current device info
    ofSetColor(200, 200, 200);
    int startY = y + 45;
    
    ofDrawBitmapString("Device: " + devicePath, x + 10, startY);
    startY += lineHeight;
    
    // Draw current format
    ofDrawBitmapString("Format: " + format, x + 10, startY);
    startY += lineHeight;
    
    // Draw resolution
    ofDrawBitmapString("Size: " + ofToString(vidWidth) + "x" + ofToString(vidHeight),
                     x + 10, startY);
    startY += lineHeight;
    
    // Draw frame rate
    ofDrawBitmapString("FPS: " + ofToString(frameRate), x + 10, startY);
    startY += lineHeight * 2;
    
    // Instructions
    ofSetColor(180, 180, 255);
    ofDrawBitmapString("Press Shift+V to cycle devices", x + 10, startY);
    startY += lineHeight;
    
    ofDrawBitmapString("Press Shift+F to cycle formats", x + 10, startY);
    startY += lineHeight;
    
    ofDrawBitmapString("Press Shift+Z to cycle resolutions", x + 10, startY);
    
    ofPopStyle();
}
