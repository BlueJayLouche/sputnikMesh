#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
    #if defined(__APPLE__) && (defined(__arm64__) || defined(__aarch64__))
        // Apple Silicon - can use desktop OpenGL
        ofGLWindowSettings settings;
        settings.setGLVersion(3, 2);
        settings.setSize(1024, 768);
        ofLogNotice("main") << "Using OpenGL 3.2 renderer for Apple Silicon";
    #elif defined(__arm__) || defined(__aarch64__)
        // Non-Apple ARM platform (Raspberry Pi, etc.)
        ofGLESWindowSettings settings;
        settings.glesVersion = 2;
        settings.setSize(640, 480);
        ofLogNotice("main") << "Using OpenGL ES2 renderer for ARM";
    #else
        // Desktop x86/x64 platform
        ofGLWindowSettings settings;
        settings.setGLVersion(3, 2);
        settings.setSize(1024, 768);
        ofLogNotice("main") << "Using OpenGL 3.2 renderer";
    #endif
    
    // Create window and initialize app
    auto window = ofCreateWindow(settings);
    ofRunApp(window, make_shared<ofApp>());
    ofRunMainLoop();
}
