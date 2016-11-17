#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    bShowGui = false;
    bUpdateBgColor = false;
    
    fullscreen = true;
    font.load("font/cooperBlack.ttf", 120 );
    
    frameW = 720; frameH = 720;
    camW = 1280; camH = 720;
    
    maskW = -( frameH * camW ) / camH;
    maskH = frameH;
    maskX = frameW + ( -maskW - frameW ) / 2;
    cout << "maskX: " << maskX << endl;
    maskY = 0;
    
    chromakey = new ofxChromaKeyShader(camW, camH);
    
    /////////////////////////
    //transparent gif layer//
    /////////////////////////
    f_max_gifs = 26;
    f_current_gif = 0;
    f_nFiles = f_dir.listDir("transparent_gifs/" + ofToString( f_current_gif ));
    if(f_nFiles) {
        for(int i=0; i<f_dir.size(); i++) {
            // add the image to thevector
            string filePath = f_dir.getPath(i);
            f_images.push_back(ofImage());
            f_images.back().load(filePath);
        }
    }
    else printf("Could not find foreground folder\n");
    cout << "f_images.size(): " << f_images.size() << endl;
    
    bMoveable.resize( f_max_gifs );
    bMoveable[ 0 ] = 0;
    bMoveable[ 1 ] = 1;
    bMoveable[ 2 ] = 0;
    bMoveable[ 3 ] = 0;
    bMoveable[ 4 ] = 0;
    bMoveable[ 5 ] = 1;
    bMoveable[ 6 ] = 1;
    bMoveable[ 7 ] = 1;
    bMoveable[ 8 ] = 1;
    bMoveable[ 9 ] = 1;
    bMoveable[ 10 ] = 1;
    bMoveable[ 11 ] = 1;
    bMoveable[ 12 ] = 1;
    bMoveable[ 13 ] = 1;
    bMoveable[ 14 ] = 1;
    bMoveable[ 15 ] = 1;
    bMoveable[ 16 ] = 1;
    bMoveable[ 17 ] = 1;
    bMoveable[ 18 ] = 1;
    bMoveable[ 19 ] = 1;
    bMoveable[ 20 ] = 1;
    bMoveable[ 21 ] = 1;
    bMoveable[ 22 ] = 1;
    bMoveable[ 23 ] = 1;
    bMoveable[ 24 ] = 1;
    bMoveable[ 25 ] = 1;
    bMoveable[ 26 ] = 1;





    
    /////////////////////////
    //background gif layer//
    /////////////////////////
    b_max_gifs = 20;
    b_current_gif = 0;
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

    
    // GUI
    chromaGui.setDefaultHeight(18);
    chromaGui.setDefaultWidth(camW/2);
    chromaGui.setup(chromakey->paramGp, "chromaSettings.xml");
    chromaGui.loadFromFile("chromaSettings.xml");
    chromaGui.setPosition(0, 0);
    
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
        chromakey->updateChromakeyMask(webcam.getTextureReference(), b_images[ b_frameIndex ].getTextureReference());

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
            ofEnableAlphaBlending();
            ofSetColor( 250, 250, 250, 100 );
            ofFill();
            ofDrawRectangle( 0, 0, ofGetWidth(), ofGetHeight());
            ofDisableAlphaBlending();
            ofSetColor( 255, 255, 255 );
        }
    }
    
    if ( capture_gif ) {
        ofSetColor( 250, 250, 250 );
        ofFill();
        ofDrawRectangle( 0, 0, ofGetWidth() / 2 - ofGetHeight() / 2, ofGetHeight());
        ofDrawRectangle( ofGetWidth() - (ofGetWidth() / 2 - ofGetHeight() / 2) , 0, ofGetWidth() / 2 - ofGetHeight() / 2, ofGetHeight());
        ofSetColor( 255, 255, 255 );
    }
    
    if ( display_gif_done ) {
        //string success = "GIF CAPTURED!";
        //string over_there = "<-----";
        font.drawString( "GIF CAPTURED!", ofGetWidth() / 2 - 700, ofGetHeight() / 2 - 100 );
        font.drawString( "<-----", ofGetWidth() / 2 - 500, ofGetHeight() / 2 + 100 );
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
            ofDrawRectangle(bgColorPos.x + camW/2, bgColorPos.y, chromakey->bgColorSize.get(), chromakey->bgColorSize.get());
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
    b_images[ b_frameIndex ].draw( 0, 0, frameW, frameH );
    
    
    // ============= cam mask =============
    ofEnableAlphaBlending();
    //chromakey->drawFinalImage(0, 0, camW, camH );
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
            if ( !startTimer && !capture_gif ) {
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
                }
            break;
            
        case OF_KEY_DOWN:
            if ( !startTimer && !capture_gif ) {
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
            }
            break;


        case OF_KEY_RIGHT:
            if ( !startTimer && !capture_gif ) {
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
            }
            break;
            
        case OF_KEY_LEFT:
            if ( !startTimer && !capture_gif ) {
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
            }
            break;
            
        case '8':

            if ( bMoveable[ f_current_gif ]) {
                f_images_y -= 10;
                if ( f_images_y <= -frameW ) {
                    f_images_y = frameW;
                }
            }

            break;
        case '2':
            if ( bMoveable[ f_current_gif ]) {
                f_images_y += 10;
                if ( f_images_y >= frameW ) {
                    f_images_y = -frameW;
                }
            }
            break;
        case '6':
            if( bMoveable[ f_current_gif ]) {
                f_images_x -= 10;
                if ( f_images_x <= -frameW ) {
                    f_images_x = frameW;
                }
            }
            break;
        case '4':
            if ( bMoveable[ f_current_gif ]) {
                f_images_x += 10;
                if ( f_images_x >= frameW ) {
                    f_images_x = -frameW;
                }
            }
            break;
            
        case '0':
            if ( !startTimer && !capture_gif ) {
                b_current_gif = (int) ofRandom( b_max_gifs );
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
                
                f_current_gif = (int) ofRandom( f_max_gifs );
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

            }
            
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