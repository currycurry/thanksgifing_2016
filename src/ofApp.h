#pragma once

#include "ofMain.h"

#include "ofxChromaKeyShader.h"
#include "ofxGui.h"
#include "ofxGifEncoder.h"
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
    float sequenceFPS;


    
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
    
    // === Foreground Images =============================
    ofDirectory f_dir;
    vector <ofImage> f_images;
    int f_current_gif;
    int f_nFiles;
    int f_images_x, f_images_y;
    int f_max_gifs;
    bool  bFrameIndependent;
    int f_frameIndex;
    int f_lastFrameIndex;

    // === Background Images =============================
    ofDirectory b_dir;
    vector <ofImage> b_images;
    int b_current_gif;
    int b_nFiles;
    int b_images_x, b_images_y;
    int b_max_gifs;
    int b_frameIndex;
    int b_lastFrameIndex;
    
    // === FBO =============================
    ofFbo fbo;
    ofPixels fbo_pix;
    void drawToFBO();
    
    // === Gif Encoder =============================
    ofxGifEncoder gifEncoder;
    void onGifSaved(string & fileName);
    void captureFrame();
    void clear_gif_buffer();
    
    bool capture_gif;
    vector <ofTexture *> txs; // for previewing
    vector <ofxGifFrame *> pxs;
    int file_number;
    float export_frame_duration;
    int nFrames;
    int totalFrames;
    int frameIndex;
    int lastFrameIndex;
    void setTotalFrames();
    
    int     picStart, doneStart;
    int     picTimer, doneTimer;
    int     picDelay, doneDelay;
    bool    startTimer;
    
    ofTrueTypeFont font;
    
    bool display_gif_done;


};
