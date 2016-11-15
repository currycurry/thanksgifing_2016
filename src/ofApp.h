#pragma once

#include "ofMain.h"

#include "ofxChromaKeyShader.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void drawDebugMasks();
    void drawCheckerboard(float x, float y, int width, int height, int size);
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    int frameW, frameH;
    bool fullscreen;

    
    // === Chroma Key =============================
    ofImage bg_image;
    
    ofxChromaKeyShader *chromakey;
    ofVideoGrabber webcam;
    int camW, camH;
    int maskW, maskH, maskX, maskY;
    
    ofFbo checkerboardTex;
    
    ofxPanel chromaGui;
    bool bShowGui;
    bool bUpdateBgColor;
    
    // === FBO =============================
    ofFbo fbo;
    ofPixels fbo_pix;
    void drawToFBO();

};
