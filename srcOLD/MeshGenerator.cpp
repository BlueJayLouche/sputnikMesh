#include "MeshGenerator.h"

MeshGenerator::MeshGenerator(ParameterManager* paramManager)
    : paramManager(paramManager), width(0), height(0) {
}

void MeshGenerator::setup(int canvasWidth, int canvasHeight) {
    width = canvasWidth;
    height = canvasHeight;
    
    // Generate initial mesh based on default settings
    updateMeshFromParameters();
}

void MeshGenerator::update() {
    // Check if mesh needs updating
    int currentScale = paramManager->getScale();
    MeshType currentMeshType = paramManager->getMeshType();
    
    if (currentScale != lastScale || currentMeshType != lastMeshType) {
        updateMeshFromParameters();
        lastScale = currentScale;
        lastMeshType = currentMeshType;
        needsUpdate = false;
    }
}

void MeshGenerator::updateMeshFromParameters() {
    int resolution = paramManager->getScale();
    bool performanceMode = paramManager->isPerformanceModeEnabled();
    
    // In performance mode, limit the resolution if it's higher than the performance scale
    if (performanceMode) {
        int performanceScale = paramManager->getPerformanceScale();
        if (resolution > performanceScale) {
            resolution = performanceScale;
        }
    }
    
    switch (paramManager->getMeshType()) {
        case MeshType::TriangleGrid:
        case MeshType::TriangleWireframe:
            generateTriangleMesh(resolution);
            break;
            
        case MeshType::HorizontalLines:
            generateHorizontalLineMesh(resolution);
            break;
            
        case MeshType::VerticalLines:
            generateVerticalLineMesh(resolution);
            break;
    }
}

void MeshGenerator::clearMesh() {
    mesh.clear();
}

void MeshGenerator::generateTriangleMesh(int resolution) {
    clearMesh();
    
    float rescale = 1.0f / resolution;
    bool performanceMode = paramManager->isPerformanceModeEnabled();
    
    // Create triangular mesh using vertices and texture coordinates
    for (int i = 0; i < resolution; i++) {
        for (int j = 0; j < resolution; j++) {
            // Calculate vertex positions
            float x0 = j * width / (float)resolution;
            float x1 = (j + 1) * width / (float)resolution;
            float y0 = i * height / (float)resolution;
            float y1 = (i + 1) * height / (float)resolution;
            
            ofVec3f vertex1(x0, y0, 0);
            ofVec3f vertex2(x1, y0, 0);
            ofVec3f vertex3(x1, y1, 0);
            ofVec3f vertex4(x0, y1, 0);
            
            // Calculate texture coordinates
            float tex_x0 = j * rescale;
            float tex_x1 = (j + 1) * rescale;
            float tex_y0 = i * rescale;
            float tex_y1 = (i + 1) * rescale;
            
            ofVec2f texCoord1(tex_x0, tex_y0);
            ofVec2f texCoord2(tex_x1, tex_y0);
            ofVec2f texCoord3(tex_x1, tex_y1);
            ofVec2f texCoord4(tex_x0, tex_y1);
            
            // First triangle (top-left, top-right, bottom-right)
            mesh.addVertex(vertex1);
            mesh.addVertex(vertex2);
            mesh.addVertex(vertex3);
            
            mesh.addTexCoord(texCoord1);
            mesh.addTexCoord(texCoord2);
            mesh.addTexCoord(texCoord3);
            
            // In performance mode, optimize by using only one triangle per cell
            if (!performanceMode || (i % 2 == 0 && j % 2 == 0)) {
                // Second triangle (bottom-right, bottom-left, top-left)
                mesh.addVertex(vertex3);
                mesh.addVertex(vertex4);
                mesh.addVertex(vertex1);
                
                mesh.addTexCoord(texCoord3);
                mesh.addTexCoord(texCoord4);
                mesh.addTexCoord(texCoord1);
            }
        }
    }
    
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    vboMesh = mesh;
}

void MeshGenerator::generateHorizontalLineMesh(int resolution) {
    clearMesh();
    
    int doubledResolution = resolution * 2;
    float rescale = 1.0f / doubledResolution;
    
    // Create horizontal line mesh
    for (int i = 0; i < doubledResolution; i++) {
        for (int j = 0; j < doubledResolution; j++) {
            // Calculate vertex positions
            int x0 = j * width / doubledResolution;
            int x1 = (j + 1) * width / doubledResolution;
            int y0 = i * height / doubledResolution;
            
            ofVec3f vertex1(x0, y0, 0);
            ofVec3f vertex2(x1, y0, 0);
            
            // Calculate texture coordinates
            float tex_x0 = j * rescale;
            float tex_x1 = (j + 1) * rescale;
            float tex_y0 = i * rescale;
            
            ofVec2f texCoord1(tex_x0, tex_y0);
            ofVec2f texCoord2(tex_x1, tex_y0);
            
            // Add line segment
            mesh.addVertex(vertex1);
            mesh.addVertex(vertex2);
            
            mesh.addTexCoord(texCoord1);
            mesh.addTexCoord(texCoord2);
        }
    }
    
    mesh.setMode(OF_PRIMITIVE_LINES);
    vboMesh = mesh;
}

void MeshGenerator::generateVerticalLineMesh(int resolution) {
    clearMesh();
    
    int doubledResolution = resolution * 2;
    float rescale = 1.0f / doubledResolution;
    
    // Create vertical line mesh
    for (int i = 0; i < doubledResolution; i++) {
        for (int j = 0; j < doubledResolution; j++) {
            // Calculate vertex positions
            int x0 = i * width / doubledResolution;
            int y0 = j * height / doubledResolution;
            int y1 = (j + 1) * height / doubledResolution;
            
            ofVec3f vertex1(x0, y0, 0);
            ofVec3f vertex2(x0, y1, 0);
            
            // Calculate texture coordinates
            float tex_x0 = i * rescale;
            float tex_y0 = j * rescale;
            float tex_y1 = (j + 1) * rescale;
            
            ofVec2f texCoord1(tex_x0, tex_y0);
            ofVec2f texCoord2(tex_x0, tex_y1);
            
            // Add line segment
            mesh.addVertex(vertex1);
            mesh.addVertex(vertex2);
            
            mesh.addTexCoord(texCoord1);
            mesh.addTexCoord(texCoord2);
        }
    }
    
    mesh.setMode(OF_PRIMITIVE_LINES);
    vboMesh = mesh;
}

ofVboMesh& MeshGenerator::getMesh() {
    return vboMesh;
}
