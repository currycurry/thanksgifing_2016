#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    bShowGui = false;
    bUpdateBgColor = true;
    
    frameW = 640; frameH = 640;
    camW = 640; camH = 480;
    fullscreen = true;

    
    maskW = (frameH * camW ) / camH;
    maskH = frameH;
    maskX = -( maskW - frameW ) / 2;
    maskY = 0;
    
    ofSetWindowShape(camW*2, camH*1.5f);
    
    chromakey = new ofxChromaKeyShader(camW, camH);
    
    // webcam setup
    webcam.setDesiredFrameRate(60);
    webcam.initGrabber(camW, camH);
    
    // maskee
    bg_image.load("bg.jpg");
    
    // GUI
    chromaGui.setDefaultHeight(18);
    chromaGui.setDefaultWidth(camW/2);
    chromaGui.setup(chromakey->paramGp, "chromaSettings.xml");
    chromaGui.loadFromFile("chromaSettings.xml");
    chromaGui.setPosition(0, 0);
    
    bUpdateBgColor = false;
    
    //fbo
    fbo.allocate( frameW, frameH, GL_RGBA );
    fbo.begin();
    ofClear(255,255,255, 0);
    fbo.end();
    
  
}

//--------------------------------------------------------------
void ofApp::exit() {
    delete chromakey;
}

//--------------------------------------------------------------
void ofApp::update(){
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    ofSetFullscreen( fullscreen );
    
    if ( fullscreen && !bShowGui ) {
        ofHideCursor();
    }
    
    else {
        ofShowCursor();
    }
    
    webcam.update();
    if(webcam.isFrameNew()) {
        if(bUpdateBgColor)
            chromakey->updateBgColor(webcam.getPixelsRef());
        chromakey->updateChromakeyMask(webcam.getTextureReference(), bg_image.getTextureReference());
    }
    
    fbo.begin();
    drawToFBO();
    fbo.end();

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground( 0 );
    
    ofEnableAlphaBlending();
    fbo.draw( ofGetWidth() / 2 - ofGetHeight() / 2, 0, ofGetHeight(), ofGetHeight());
    ofDisableAlphaBlending();
    
    // GUI
    if(bShowGui) {
        
        chromaGui.draw();
        //drawDebugMasks();
        
        // draw bg color's reference Rect
        if(bUpdateBgColor) {
            ofPushStyle();
            ofNoFill();
            ofSetLineWidth(3);
            ofSetColor(255);
            ofVec2f bgColorPos = chromakey->bgColorPos.get();
            ofRect(bgColorPos.x + camW/2, bgColorPos.y, chromakey->bgColorSize.get(), chromakey->bgColorSize.get());
            ofDrawBitmapString("bgColor", bgColorPos.x + camW/2, bgColorPos.y - 5);
            ofPopStyle();
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::drawToFBO(){
    
    ofSetColor(255);
    ofBackground(0);
    
    //background
    bg_image.draw(0, 0, frameW, frameH );
    
    // draw Cam mask
    ofEnableAlphaBlending();
    //chromakey->drawFinalImage(0, 0, camW, camH);
    chromakey->drawFinalImage( maskX, maskY, maskW, maskH);
    ofDisableAlphaBlending();
    
    //foreground
    

    
}

//--------------------------------------------------------------
void ofApp::drawDebugMasks() {
    ofSetColor(255);
    int previewW = camW/2, previewH = camH/2, labelOffset = 10;
    
    chromakey->drawBaseMask(camW + previewW, 0, previewW, previewH);
    ofDrawBitmapStringHighlight("Base mask", camW + previewW, labelOffset, ofColor(0, 125), ofColor::yellowGreen);
    
    chromakey->drawDetailMask(camW + previewW, previewH, previewW, previewH);
    ofDrawBitmapStringHighlight("Detailed mask", camW + previewW, previewH + labelOffset, ofColor(0, 125), ofColor::yellowGreen);
    
    chromakey->drawChromaMask(previewW, camH, previewW, previewH);
    ofDrawBitmapStringHighlight("Chroma mask", previewW, camH + labelOffset, ofColor(0, 125), ofColor::yellowGreen);
				
    drawCheckerboard(camW, camH, previewW, previewH, 5);
    chromakey->drawFinalMask(camW, camH, previewW, previewH);
    ofDrawBitmapStringHighlight("Final mask", camW, camH + labelOffset, ofColor(0, 125), ofColor::yellowGreen);
    
    webcam.draw(camW + previewW, camH, previewW, previewH);
    ofDrawBitmapStringHighlight("RGB image", camW + previewW, camH + labelOffset, ofColor(0, 125), ofColor::yellowGreen);
}

//--------------------------------------------------------------
void ofApp::drawCheckerboard(float x, float y, int width, int height, int size) {
    if (!checkerboardTex.isAllocated()) {
        checkerboardTex.allocate(width, height);
        
        ofPushStyle();
        checkerboardTex.begin();
        ofClear(255, 255, 255, 255);
        int numWidth = width/size;
        int numHeight = height/size;
        for(int h=0; h<numHeight; h++) {
            for(int w=0; w<numWidth; w++) {
                if ((h+w)%2 == 0) {
                    ofSetColor(ofColor::black);
                    ofDrawRectangle(w*size, h*size, size, size);
                }
            }
        }
        checkerboardTex.end();
        ofPopStyle();
    }
    
    ofSetColor(255, 255);
    checkerboardTex.draw(x, y);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch ( key ) {
            case 'f':
            fullscreen = !fullscreen;
            break;
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    switch(key){
        case 'a':
            bUpdateBgColor = !bUpdateBgColor;
            break;
        case 'g':
            bShowGui = !bShowGui;
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}