#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    bShowGui = false;
    bUpdateBgColor = true;
    bFrameIndependent = true;
    
    frameW = 640; frameH = 640;
    camW = 640; camH = 480;
    fullscreen = true;

    
    maskW = (frameH * camW ) / camH;
    maskH = frameH;
    maskX = -( maskW - frameW ) / 2;
    maskY = 0;
    
    ofSetWindowShape(camW*2, camH*1.5f);
    
    chromakey = new ofxChromaKeyShader(camW, camH);
    
    /////////////////////////
    //transparent gif layer//
    /////////////////////////
    current_gif = 0;
    max_gifs = 5;
    
    nFiles = dir.listDir("transparent_gifs/" + ofToString( current_gif ));
    if(nFiles) {
        for(int i=0; i<dir.size(); i++) {
            // add the image to the vector
            string filePath = dir.getPath(i);
            images.push_back(ofImage());
            images.back().loadImage(filePath);
        }
    }
    
    else printf("Could not find folder\n");
    
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
    
    //gif encoder
    sequenceFPS = 5;
    file_number = ofGetUnixTime();
    export_frame_duration = 1 / sequenceFPS;
    gifEncoder.setup(frameW, frameH, export_frame_duration, 256);
    ofAddListener(ofxGifEncoder::OFX_GIF_SAVE_FINISHED, this, &ofApp::onGifSaved);
    nFrames = 0;
    totalFrames = images.size();
    cout << "totalFrames: " << totalFrames << endl;
    capture_gif = false;
    frameIndex = 0;
    lastFrameIndex = 0;
    
    startTimer = false;
    picTimer = 0;
    picDelay = 2000;
    picStart = 0;
    
    display_gif_done = false;
    doneTimer = 0;
    doneDelay = 3000;
    doneStart = 0;

    
  
}

//--------------------------------------------------------------
void ofApp::exit() {
    delete chromakey;
    gifEncoder.exit();

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
    
    
    // start gif
    if ( startTimer ) {
        picTimer = ofGetElapsedTimeMillis() - picStart;
        
        if ( picTimer >= picDelay && picTimer != 0 ) {
            cout << "timer up" << endl;
            capture_gif = true;
            startTimer = false;
        }
    }
    
    if ( display_gif_done ) {
        doneTimer = ofGetElapsedTimeMillis() - doneStart;
        
        if ( doneTimer >= doneDelay && doneTimer != 0 ) {
            cout << "done timer up" << endl;
            display_gif_done = false;
        }
    }


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
    
    // ============= cam mask =============
    ofEnableAlphaBlending();
    //chromakey->drawFinalImage(0, 0, camW, camH);
    chromakey->drawFinalImage( maskX, maskY, maskW, maskH);
    ofDisableAlphaBlending();
    
    // ============= foreground =============
    ofEnableAlphaBlending();
    // we need some images if not return
    if((int)images.size() <= 0) {
        ofSetColor(255);
        ofDrawBitmapString("No Images...", 150, ofGetHeight()/2);
        return;
    }
    
    frameIndex = 0;
    
    if(bFrameIndependent) {
        // calculate the frame index based on the app time
        // and the desired sequence fps. then mod to wrap
        frameIndex = (int)( ofGetElapsedTimef() * sequenceFPS ) % images.size();
    }
    else {
        // set the frame index based on the app frame
        // count. then mod to wrap.
        frameIndex = ofGetFrameNum() % images.size();
    }
    
    // draw the image sequence at the new frame count
    ofSetColor(255);
    images[ frameIndex ].draw( images_x, images_y, frameW, frameH );
    
    if ( capture_gif && lastFrameIndex != frameIndex ) {
        cout << "capturing" << endl;
        captureFrame();
        nFrames ++;
        
        if ( nFrames >= totalFrames ) {
            cout << "finishing capture" << endl;
            capture_gif = false;
            cout <<"start saving\n" << endl;
            file_number = ofGetUnixTime();
            gifEncoder.save("output/gif_" + ofToString( file_number ) + ".gif");
            
            
            
        }
    }
    
    lastFrameIndex = frameIndex;
    ofDisableAlphaBlending();

    

    
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
void ofApp::captureFrame() {
    
    fbo.readToPixels( fbo_pix );
    unsigned char* char_pix;
    char_pix = (unsigned char*) malloc(sizeof(unsigned char) * fbo_pix.size());
    for ( int i = 0; i < fbo_pix.size(); i ++ ) {
        char_pix[ i ] = fbo_pix[ i ];
    }
    
    
    gifEncoder.addFrame(
                        char_pix,
                        fbo.getWidth(),
                        fbo.getHeight(),
                        32,
                        1 / sequenceFPS
                        ); //.1f
    
    ofTexture * tx = new ofTexture();
    tx->allocate(frameW, frameH, GL_RGB);
    tx->loadData(char_pix, frameW, frameH, GL_RGB);
    txs.push_back(tx);
    
    free( char_pix );
    
}

//--------------------------------------------------------------
void ofApp::onGifSaved(string &fileName) {
    display_gif_done = true;
    doneStart = ofGetElapsedTimeMillis();
    cout << "gif saved as " << fileName << endl;
    clear_gif_buffer();
    doneStart = ofGetElapsedTimeMillis();
    doneTimer = 0;
    
}

//--------------------------------------------------------------
void ofApp::clear_gif_buffer() {
    
    txs.clear();
    txs.resize( 0 );
    pxs.clear();
    pxs.resize( 0 );
    nFrames = 0;
    gifEncoder.reset();
    
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch ( key ) {
        case 'f':
            fullscreen = !fullscreen;
            break;
            
        case ' ':
            picStart = ofGetElapsedTimeMillis();
            startTimer = true;
            break;
            
        case OF_KEY_RIGHT:
            current_gif += 1;
            if ( current_gif > max_gifs ) {
                current_gif = 0;
            }
            images.clear();
            //sequenceFPS = frame_rates[ current_gif ];
            
            nFiles = dir.listDir("transparent_gifs/" + ofToString( current_gif ));
            if(nFiles) {
                
                for(int i=0; i<dir.numFiles(); i++) {
                    string filePath = dir.getPath(i);
                    images.push_back(ofImage());
                    images.back().loadImage(filePath);
                }
            }
            
            else printf("Could not find folder\n");
            totalFrames = images.size();
            images_x = 0;
            images_y = 0;
            cout << "totalFrames: " << totalFrames << endl;
            
            
            cout << "current_gif: " << current_gif << endl;
            break;
            
        case OF_KEY_LEFT:
            current_gif -= 1;
            if ( current_gif <= 0 ) {
                current_gif = max_gifs;
            }
            images.clear();
            //sequenceFPS = frame_rates[ current_gif ];
            
            nFiles = dir.listDir("transparent_gifs/" + ofToString( current_gif ));
            if(nFiles) {
                for(int i=0; i<dir.numFiles(); i++) {
                    string filePath = dir.getPath(i);
                    images.push_back(ofImage());
                    images.back().loadImage(filePath);
                }
            }
            else printf("Could not find folder\n");
            totalFrames = images.size();
            cout << "totalFrames: " << totalFrames << endl;
            
            break;
            
        case 't':
            bFrameIndependent = !bFrameIndependent;
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