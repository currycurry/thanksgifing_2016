#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    bShowGui = false;
    bUpdateBgColor = true;
    bFrameIndependent = true;
    
    frameW = 640; frameH = 640;
    camW = 640; camH = 480;
    fullscreen = true;
    
    font.loadFont("font/cooperBlack.ttf", 80 );
    
    maskW = (frameH * camW ) / camH;
    maskH = frameH;
    maskX = -( maskW - frameW ) / 2;
    maskY = 0;
    
    ofSetWindowShape(camW*2, camH*1.5f);
    
    chromakey = new ofxChromaKeyShader(camW, camH);
    
    /////////////////////////
    //transparent gif layer//
    /////////////////////////
    
    f_current_gif = 0;
    f_max_gifs = 5;
    f_nFiles = f_dir.listDir("transparent_gifs/" + ofToString( f_current_gif ));
    if(f_nFiles) {
        for(int i=0; i<f_dir.size(); i++) {
            // add the image to the vector
            string filePath = f_dir.getPath(i);
            f_images.push_back(ofImage());
            f_images.back().load(filePath);
        }
    }
    else printf("Could not find foreground folder\n");
    cout << "f_images.size(): " << f_images.size() << endl;

    
    /////////////////////////
    //background gif layer//
    /////////////////////////
    
    b_current_gif = 0;
    b_max_gifs = 6;
    b_nFiles = b_dir.listDir("background_gifs/" + ofToString( b_current_gif ));
    if(b_nFiles) {
        for(int i=0; i<b_dir.size(); i++) {
            // add the image to the vector
            string filePath = b_dir.getPath(i);
            b_images.push_back(ofImage());
            b_images.back().load(filePath);
        }
    }
    else printf("Could not find background folder\n");
    cout << "b_images.size(): " << b_images.size() << endl;

    
    ////////////////
    //mask  layer//
    ///////////////
    //webcam.setDesiredFrameRate(60);
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
    fbo.allocate( frameW, frameH, GL_RGB );
    fbo.begin();
    ofClear(255,255,255);
    fbo.end();
    
    //gif encoder
    sequenceFPS = 5;
    file_number = ofGetUnixTime();
    export_frame_duration = 1 / sequenceFPS;
    gifEncoder.setup(frameW, frameH, export_frame_duration, 256);
    ofAddListener(ofxGifEncoder::OFX_GIF_SAVE_FINISHED, this, &ofApp::onGifSaved);
    nFrames = 0;
    
    if ( f_images.size() > b_images.size() ) {
        totalFrames = f_images.size();
    }
    else {
        totalFrames = b_images.size();
    }
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
    
    if ( startTimer ) {
        if ( picTimer < picDelay / 6 || ( picTimer > 2 * picDelay / 6 && picTimer < 3 * picDelay / 6 ) || ( picTimer > 4 * picDelay / 6 && picTimer < 5 * picDelay / 6 ) ) {
            ofSetColor( 250, 214, 147 );
            ofFill();
            
            ofRect( 0, 0, ofGetWidth() / 2 - ofGetHeight() / 2, ofGetHeight());
            ofRect( ofGetWidth() - (ofGetWidth() / 2 - ofGetHeight() / 2) , 0, ofGetWidth() / 2 - ofGetHeight() / 2, ofGetHeight());
            ofSetColor( 255, 255, 255 );
        }
    }
    
    if ( capture_gif ) {
        ofSetColor( 222, 40, 57 );
        ofFill();
        ofRect( 0, 0, ofGetWidth() / 2 - ofGetHeight() / 2, ofGetHeight());
        ofRect( ofGetWidth() - (ofGetWidth() / 2 - ofGetHeight() / 2) , 0, ofGetWidth() / 2 - ofGetHeight() / 2, ofGetHeight());
        ofSetColor( 255, 255, 255 );
    }
    
    if ( display_gif_done ) {
        font.drawString( "GIF CAPTURED!", ofGetWidth() / 2- 400, ofGetWidth() / 2 - 100 );
        font.drawString( "<-----", ofGetWidth() / 2 - 200, ofGetWidth() / 2 + 100 );
    }

    
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
    
    //whichever sequence has more frames determines how long the output gif will be
    f_frameIndex = (int)( ofGetElapsedTimef() * sequenceFPS ) % f_images.size();
    b_frameIndex = (int)( ofGetElapsedTimef() * sequenceFPS ) % b_images.size();
    
    if ( f_frameIndex >= b_frameIndex ) {
        frameIndex = f_frameIndex;
    }
    else {
        frameIndex = b_frameIndex;
    }
    
    // ============= background =============
    if((int)b_images.size() <= 0) {
        ofSetColor(255);
        ofDrawBitmapString("No Background Images...", 150, ofGetHeight()/2);
        return;
    }
    // draw the image sequence at the new frame count
    ofSetColor(255);
    b_images[ b_frameIndex ].draw( b_images_x, b_images_y, frameW, frameH );
    
    
    // ============= cam mask =============
    ofEnableAlphaBlending();
    //chromakey->drawFinalImage(0, 0, camW, camH);
    chromakey->drawFinalImage( maskX, maskY, maskW, maskH);
    ofDisableAlphaBlending();
    
    // ============= foreground =============
    ofEnableAlphaBlending();
    // we need some images if not return
    if((int)f_images.size() <= 0) {
        ofSetColor(255);
        ofDrawBitmapString("No Images...", 150, ofGetHeight()/2);
        return;
    }
    
       // draw the image sequence at the new frame count
    ofSetColor(255);
    f_images[ f_frameIndex ].draw( f_images_x, f_images_y, frameW, frameH );
    
    
    // ============= gif encoder =============

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
    
    f_lastFrameIndex = f_frameIndex;
    b_lastFrameIndex = b_frameIndex;
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
                        24,
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
            
            
        case OF_KEY_UP:
            b_current_gif += 1;
            if ( b_current_gif > b_max_gifs ) {
                b_current_gif = 0;
            }
            b_images.clear();
            
            b_nFiles = b_dir.listDir("background_gifs/" + ofToString( b_current_gif ));
            if(b_nFiles) {
                
                for(int i=0; i<b_dir.size(); i++) {
                    string filePath = b_dir.getPath(i);
                    b_images.push_back(ofImage());
                    b_images.back().load(filePath);
                }
            }
            else printf("Could not find background folder\n");
            setTotalFrames();
            cout << "b_current_gif: " << b_current_gif << endl;
            break;
            
        case OF_KEY_DOWN:
            b_current_gif -= 1;
            if ( b_current_gif <= 0 ) {
                b_current_gif = b_max_gifs;
            }
            b_images.clear();
            
            b_nFiles = b_dir.listDir("background_gifs/" + ofToString( b_current_gif ));
            if(b_nFiles) {
                for(int i=0; i<b_dir.size(); i++) {
                    string filePath = b_dir.getPath(i);
                    b_images.push_back(ofImage());
                    b_images.back().load(filePath);
                }
            }
            else printf("Could not find background folder\n");
            setTotalFrames();
            cout << "b_current_gif: " << b_current_gif << endl;
            break;


        case OF_KEY_RIGHT:
            f_current_gif += 1;
            if ( f_current_gif > f_max_gifs ) {
                f_current_gif = 0;
            }
            f_images.clear();
            
            f_nFiles = f_dir.listDir("transparent_gifs/" + ofToString( f_current_gif ));
            if(f_nFiles) {
                for(int i=0; i<f_dir.size(); i++) {
                    string filePath = f_dir.getPath(i);
                    f_images.push_back(ofImage());
                    f_images.back().load(filePath);
                }
            }
            else printf("Could not find foreground folder\n");
            setTotalFrames();
            f_images_x = 0;
            f_images_y = 0;
            cout << "f_current_gif: " << f_current_gif << endl;
            break;
            
        case OF_KEY_LEFT:
            f_current_gif -= 1;
            if ( f_current_gif <= 0 ) {
                f_current_gif = f_max_gifs;
            }
            f_images.clear();
            f_nFiles = f_dir.listDir("transparent_gifs/" + ofToString( f_current_gif ));
            if(f_nFiles) {
                for(int i=0; i<f_dir.size(); i++) {
                    string filePath = f_dir.getPath(i);
                    f_images.push_back(ofImage());
                    f_images.back().load(filePath);
                }
            }
            else printf("Could not find foreground folder\n");
            setTotalFrames();
            f_images_x = 0;
            f_images_y = 0;
            cout << "f_current_gif: " << f_current_gif << endl;
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
void ofApp::setTotalFrames(){

    if ( f_images.size() > b_images.size() ) {
        totalFrames = f_images.size();
    }
    else {
        totalFrames = b_images.size();
    }
    cout << "total frames set to: " << totalFrames << endl;
    
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