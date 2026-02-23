#pragma once

#include "ofMain.h"
#include "ParameterManager.h"

class MeshGenerator {
public:
    MeshGenerator(ParameterManager* paramManager);
    
    // Core methods
    void setup(int width, int height);
    void update();
    
    // Mesh generation methods
    void generateTriangleMesh(int resolution);
    void generateHorizontalLineMesh(int resolution);
    void generateVerticalLineMesh(int resolution);
    
    // Mesh access
    ofVboMesh& getMesh();
    
    // Updates mesh based on current parameter settings
    void updateMeshFromParameters();
    
private:
    // Canvas dimensions
    int width;
    int height;
    
    // Mesh data
    ofMesh mesh;
    ofVboMesh vboMesh;
    
    // Reference to parameter manager
    ParameterManager* paramManager;
    
    // Helper methods
    void clearMesh();
    void setupTriangleMesh(int resolution);
    void setupHorizontalLineMesh(int resolution);
    void setupVerticalLineMesh(int resolution);
    
    bool needsUpdate;
    int lastScale;
    MeshType lastMeshType;
};
